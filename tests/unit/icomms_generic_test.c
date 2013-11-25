#include <test.h>
#include <icomms_generic.h>
#include <imessage.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

static char first_test_string[] = "This is the first test string";
static char second_test_string[] = "This is the second test string";
static char server_path[] = "/tmp/imessage_server";
static char test_file[] = "/tmp/imessage_test_file_XXXXXX";
static int server_socket = -1;

static int create_server(void)
{
    struct sockaddr_un server_address;

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        return -1;
    }
    memset(&server_address, 0, sizeof(struct sockaddr_un));
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, server_path, sizeof(server_address.sun_path)-1);

    if (bind(server_socket, (struct sockaddr *)&server_address,
             sizeof(struct sockaddr_un)) < 0)
    {
        return -1;
    }

    if (listen(server_socket, 10) < 0)
    {
        return -1;
    }

    return 0;
}

static int delete_server(void)
{
    if (server_socket > 0)
    {
        close (server_socket);
    }
    return 0;
}

static void test_message_passing(void)
{
    /*
     * This test sends messages between two processes.
     * The child process calls execve to make sure we do not
     * reuse the same file descriptors, otherwise the file descriptor
     * sharing is pointless.
     */
    pid_t child = 0;
    int channel[2];

    assert_int_equal(0, pipe(channel));
    child = fork();
    assert_int_not_equal(-1, child);
    if (child == 0)
    {
        /* Child */
        fd_set rfds;
        struct timeval tv;

        close(channel[1]);
        FD_ZERO(&rfds);
        FD_SET(channel[0], &rfds);
        /* Safeguard in case the parent process dies or has problems */
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        int number = select(channel[0]+1, &rfds, NULL, NULL, &tv);
        if(0 == number)
        {
            /* parent did not reply, leaving */
            exit(0);
        }
        /* Ready to rock and roll, call execve */
        char *argv[] = { "imessage_test_helper", server_path, NULL };
        char *envp[] = { NULL };
        execve("imessage_test_helper", argv, envp);
        exit(1);
    }
    else
    {
        /* Parent */
        close (channel[0]);
        /* Create the server */
        assert_int_equal(0, create_server());
        /* Tell the child to start */
        int message = 0;
        write(channel[1], &message, sizeof(message));

        /* Start accepting connections */
        fd_set rfds;
        struct timeval tv;

        FD_ZERO(&rfds);
        FD_SET(server_socket, &rfds);
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        int number = select(channel[1]+1, &rfds, NULL, NULL, &tv);
        assert_int_equal(1, number);

        /* Accept the connection */
        struct sockaddr_un client_address;
        socklen_t client_address_size = sizeof(struct sockaddr_un);
        int client_socket = accept(server_socket, (struct sockaddr_un *)&client_address,
                                   &client_address_size);
        assert_int_not_equal(-1, client_socket);

        /* We are connected, run the test */
        ICommsInterface *interface = ICommsInterfaceNew(client_socket);
        int fd = mktemp(test_file);
        assert_int_not_equal(-1, fd);
        /* Send the file descriptor to the child process */
        IMessage *message = IMessageNew(IMessage_ShareOwnership, &fd);
        assert_int_not_equal(-1, ICommsInterfaceWrite(interface, message));

        /* Wait for the process to be ready with the file descriptor */
        IMessage *reply = NULL;
        assert_int_not_equal(-1, ICommsInterfaceRead(interface, &reply));
        /* Check the reply */
        int expected_length = strlen(second_test_string);
        assert_int_equal(expected_length, IMessageDataLength(reply));
        assert_string_equal(second_test_string, (char *)IMessageDataContent(reply));
        /* Check the file */
        assert_int_equal(0, lseek(fd, 0, SEEK_SET));
        char file_content[128];
        assert_int_equal(strlen(first_test_string), read(fd, file_content, strlen(first_test_string)));
        assert_string_equal(file_content, first_test_string);
        /* Delete the file */
        close (fd);
        unlink (test_file);
        /* Destroy the server */
        assert_int_equal(0, delete_server());
        /* Wait for the child process */
        int status = 0;
        waitpid(child, &status, 0);
        assert_true(WIFEXITED(status));
        assert_int_equal(0, WEXITSTATUS(status));
    }
}

static void test_basic(void)
{
    ICommsInterface *icomms = NULL;

    assert_true(ICommsInterfaceNew(-1) == NULL);
    ICommsInterfaceDestroy(NULL);
    ICommsInterfaceDestroy(&icomms);
    assert_int_equal(-1, ICommsInterfaceTimeout(NULL));
    ICommsInterfaceSetTimeout(NULL, 0);
    assert_int_equal(-1, ICommsInterfaceWrite(NULL, NULL));
    assert_int_equal(-1, ICommsInterfaceRead(NULL, NULL));

    assert_true((icomms = ICommsInterfaceNew(0)) != NULL);
    assert_int_equal(-1, ICommsInterfaceWrite(icomms, NULL));
    assert_int_equal(-1, ICommsInterfaceRead(icomms, NULL));
    assert_int_equal(ICOMMS_INTERFACE_DEFAULT_TIMEOUT, ICommsInterfaceTimeout(icomms));
    ICommsInterfaceSetTimeout(icomms, -1);
    assert_int_equal(ICOMMS_INTERFACE_DEFAULT_TIMEOUT, ICommsInterfaceTimeout(icomms));
    ICommsInterfaceSetTimeout(icomms, 42);
    assert_int_equal(42, ICommsInterfaceTimeout(icomms));

    ICommsInterfaceDestroy(&icomms);
    assert_true(icomms == NULL);
}

int main()
{
    PRINT_TEST_BANNER();
    const UnitTest tests[] =
    {
        unit_test(test_basic)
    };

    return run_tests(tests);
}

