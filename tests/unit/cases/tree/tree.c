#include "base/util.h"
#include "cthulhu/tree/ops.h"
#include "unit/ct-test.h"

#include "arena/arena.h"

#include "setup/memory.h"

#include "std/str.h"

int main(void)
{
    test_install_panic_handler();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("tree", arena);

    {
        test_group_t formatting = test_group(&suite, "formatting");

        GROUP_EXPECT_PASS(formatting, "none quals is empty", str_equal("", quals_string(eQualNone)));
        GROUP_EXPECT_PASS(formatting, "const quals is const", str_equal("const", quals_string(eQualConst)));
    }

    return test_suite_finish(&suite);
}
