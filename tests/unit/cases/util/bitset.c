#include "unit/ct-test.h"

#include "setup/memory.h"

#include "base/bitset.h"

int main(void)
{
    test_install_panic_handler();
    test_install_electric_fence();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("bitset", arena);

    {
        test_group_t group = test_group(&suite, "construction");
        char data[8];
        bitset_t a = CT_BITSET_ARRAY(data);
        GROUP_EXPECT_PASS(group, "length >= 3", bitset_len(a) >= 3);
    }

    {
        test_group_t group = test_group(&suite, "set");
        char data[8];
        bitset_t a = CT_BITSET_ARRAY(data);
        bitset_set(a, 0);
        GROUP_EXPECT_PASS(group, "set bit 0", bitset_test(a, 0));
        bitset_set(a, 1);
        GROUP_EXPECT_PASS(group, "set bit 1", bitset_test(a, 1));
        bitset_set(a, 2);
        GROUP_EXPECT_PASS(group, "set bit 2", bitset_test(a, 2));
    }

    {
        test_group_t group = test_group(&suite, "clear");
        char data[8];
        bitset_t a = CT_BITSET_ARRAY(data);
        bitset_set(a, 0);
        bitset_set(a, 1);
        bitset_set(a, 2);
        bitset_clear(a, 0);
        GROUP_EXPECT_PASS(group, "clear bit 0", !bitset_test(a, 0));
        bitset_clear(a, 1);
        GROUP_EXPECT_PASS(group, "clear bit 1", !bitset_test(a, 1));
        bitset_clear(a, 2);
        GROUP_EXPECT_PASS(group, "clear bit 2", !bitset_test(a, 2));
    }

    return test_suite_finish(&suite);
}
