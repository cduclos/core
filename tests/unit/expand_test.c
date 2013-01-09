#include "test.h"
#include "expand.h"
#include "expand.c"

static void GetNaked_test(void **state)
{
    char destination[CF_MAXVARSIZE];
    /*
     * The test cases:
     * undecorated: a variable that is already naked, therefore we should get exactly the same variable.
     * decorated: a variable that needs to be stripped out of its decorations.
     * invalid: a string that contains no valid variable.
     */
    char undecorated[] = "undecorated";
    char decorated[] = "@(decorated)";
    char invalid[] = "@()";
    char wrongCharactersDecorated[] = "@(#&&)";
    char wrongCharactersUndecorated[] = "#&&";

    /*
     * First check the decorated path
     */
    memset(destination, 0, CF_MAXVARSIZE);
    assert_int_equal(0, GetNaked(destination, decorated));
    assert_string_equal(destination, "decorated");
    /*
     * Check the undecorated path
     */
    memset(destination, 0, CF_MAXVARSIZE);
    assert_int_equal(0, GetNaked(destination, undecorated));
    assert_string_equal(destination, "undecorated");
    /*
     * Check the invalid path
     */
    memset(destination, 0, CF_MAXVARSIZE);
    assert_int_equal(-1, GetNaked(destination, invalid));
    assert_int_equal(0, strlen(destination));
    /*
     * Check wrong characters but properly formed
     */
    memset(destination, 0, CF_MAXVARSIZE);
    assert_int_equal(-1, GetNaked(destination, wrongCharactersDecorated));
    assert_int_equal(0, strlen(destination));
    /*
     * Check wrong characters and not decorated
     */
    memset(destination, 0, CF_MAXVARSIZE);
    assert_int_equal(-1, GetNaked(destination, wrongCharactersUndecorated));
    assert_int_equal(0, strlen(destination));
}

int main()
{
    const UnitTest tests[] = {
        GetNaked_test
    };
    return run_tests(tests);
}
