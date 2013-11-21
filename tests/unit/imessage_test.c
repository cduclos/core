#include <test.h>
#include <imessage.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

static void test_basic(void)
{
    int data = 0x55AA55AA;

    assert_true(IMessageNew((IMessageRequests)-1, &data) == NULL);
    assert_true(IMessageNew(IMessage_Invalid, NULL) == NULL);

    assert_true(IMessageDataContent(NULL) == NULL);
    assert_true(IMessageDataLength(NULL) == 0);
    assert_true(IMessageRequest(NULL) == IMessage_Invalid);
    assert_true(IMessageSender(NULL) == 0);
}

static void test_shareownership_message(void)
{
    IMessage *message = NULL;
    int data = 0x55AA55AA;
    int *pdata = NULL;

    assert_true((message = IMessageNew(IMessage_ShareOwnership, &data)) != NULL);
    assert_true((pdata = IMessageDataContent(message)) != NULL);
    assert_int_equal(*pdata, data);
    assert_int_equal(sizeof(int), IMessageDataLength(message));
    assert_int_equal(getpid(), IMessageSender(message));
    assert_int_equal(IMessage_ShareOwnership, IMessageRequest(message));

    IMessageDestroy(&message);
    assert_true(message == NULL);
}

static void test_text_message(void)
{
    IMessage *message = NULL;
    char data[] = "This is a nice text";
    char *pdata = NULL;

    assert_true((message = IMessageNew(IMessage_WriteText, data)) != NULL);
    assert_true((pdata = IMessageDataContent(message)) != NULL);
    assert_string_equal(pdata, data);
    assert_int_equal(strlen(data), IMessageDataLength(message));
    assert_int_equal(getpid(), IMessageSender(message));
    assert_int_equal(IMessage_WriteText, IMessageRequest(message));

    IMessageDestroy(&message);
    assert_true(message == NULL);
}

int main()
{
    PRINT_TEST_BANNER();
    const UnitTest tests[] =
    {
        unit_test(test_basic),
        unit_test(test_shareownership_message),
        unit_test(test_text_message)
    };

    return run_tests(tests);
}
