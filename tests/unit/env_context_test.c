#include "test.h"
#include "env_context.h"

static void IsDefinedClass_test(void **state)
{
    /*
     * Try some simple tests with NULL pointers
     */

}

#if 0
bool IsDefinedClass(const char *class, const char *ns);
#endif

int main()
{
    const UnitTest tests[] = {
        unit_test(IsDefinedClass_test)
    };
    return run_tests(tests);
}

