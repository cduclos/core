#include <test.h>
#include <icomms_generic.h>
#include <imessage.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

static char first_test_string[] = "This is the first test string";
static char second_test_string[] = "This is the second test string";
static char server_path[] = "/tmp/imessage_server";

static int create_server(void)
{
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
        create_server();
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

