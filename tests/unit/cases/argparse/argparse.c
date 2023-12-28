#include "config/config.h"
#include "core/macros.h"
#include "memory/memory.h"
#include "unit/ct-test.h"

#include "argparse/argparse.h"

#include "std/vector.h"
#include "std/str.h"

typedef struct test_config_t
{
    config_t *root;

    cfg_field_t *bool_field;
    cfg_field_t *int_field;
    cfg_field_t *string_field;
    cfg_field_t *enum_field;
    cfg_field_t *flag_field;
} test_config_t;

static const cfg_info_t kRootInfo = {
    .name = "test",
    .brief = "Test config",
};

static const char *const kBoolInfoShortArgs[] = {"b", NULL};

static const cfg_info_t kBoolInfo = {
    .name = "bool",
    .brief = "A boolean field",
    .short_args = kBoolInfoShortArgs,
};

static const char *const kIntInfoShortArgs[] = {"i", NULL};

static const cfg_info_t kIntInfo = {
    .name = "int",
    .brief = "An integer field",
    .short_args = kIntInfoShortArgs,
};

static const char *const kStringInfoShortArgs[] = {"s", NULL};

static const cfg_info_t kStringInfo = {
    .name = "string",
    .brief = "A string field",
    .short_args = kStringInfoShortArgs,
};

static const char *const kEnumInfoShortArgs[] = {"e", NULL};

static const cfg_info_t kEnumInfo = {
    .name = "enum",
    .brief = "An enum field",
    .short_args = kEnumInfoShortArgs,
};

static const char *const kFlagInfoShortArgs[] = {"f", NULL};

static const cfg_info_t kFlagInfo = {
    .name = "flag",
    .brief = "A flag field",
    .short_args = kFlagInfoShortArgs,
};

enum test_enum_t
{
    eTestEnumA,
    eTestEnumB,
    eTestEnumC,
};

enum test_flags_t
{
    eTestFlagNone = 0,
    eTestFlagA = 1 << 0,
    eTestFlagB = 1 << 1,
    eTestFlagC = 1 << 2,
};

static const cfg_choice_t kEnumChoices[] = {
    {.text = "a", .value = eTestEnumA},
    {.text = "b", .value = eTestEnumB},
    {.text = "c", .value = eTestEnumC},
};

static const cfg_choice_t kFlagChoices[] = {
    {.text = "a", .value = eTestFlagA},
    {.text = "b", .value = eTestFlagB},
    {.text = "c", .value = eTestFlagC},
};

static test_config_t make_config(arena_t *arena)
{
    test_config_t config = {0};

    config.root = config_new(arena, &kRootInfo);

    cfg_bool_t bool_initial = {.initial = false};
    config.bool_field = config_bool(config.root, &kBoolInfo, bool_initial);

    cfg_int_t int_initial = {.initial = 0};
    config.int_field = config_int(config.root, &kIntInfo, int_initial);

    cfg_string_t string_initial = {.initial = NULL};
    config.string_field = config_string(config.root, &kStringInfo, string_initial);

    cfg_enum_t enum_initial = {
        .initial = eTestEnumA,
        .options = kEnumChoices,
        .count = (sizeof(kEnumChoices) / sizeof(cfg_choice_t))
    };
    config.enum_field = config_enum(config.root, &kEnumInfo, enum_initial);

    cfg_flags_t flag_initial = {
        .initial = eTestFlagNone,
        .options = kFlagChoices,
        .count = (sizeof(kFlagChoices) / sizeof(cfg_choice_t))
    };
    config.flag_field = config_flags(config.root, &kFlagInfo, flag_initial);

    return config;
}

int main(void)
{
    test_install_panic_handler();
    test_install_electric_fence();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("argparse", arena);

    // smoke test to make sure it parses at all
    {
        test_group_t group = test_group(&suite, "argparse");
        test_config_t cfg = make_config(arena);
        ap_t *ap = ap_new(cfg.root, arena);
        int result = ap_parse(ap, "-b -i 42 -s hello -e b -f a -f c");
        GROUP_EXPECT_PASS(group, "smoke test parses", result == EXIT_OK);
        GROUP_EXPECT_PASS(group, "no unknown args", vector_len(ap_get_unknown(ap)) == 0);

        GROUP_EXPECT_PASS(group, "bool", cfg_bool_value(cfg.bool_field));
        GROUP_EXPECT_PASS(group, "int", cfg_int_value(cfg.int_field) == 42);

        const char *str = cfg_string_value(cfg.string_field);
        GROUP_EXPECT_PASS(group, "string", str_equal(str, "hello"));

        enum test_enum_t e = cfg_enum_value(cfg.enum_field);
        GROUP_EXPECT_PASS(group, "enum", e == eTestEnumB);

        enum test_flags_t f = cfg_flags_value(cfg.flag_field);
        GROUP_EXPECT_PASS(group, "flag", f == (eTestFlagA | eTestFlagC));
    }

    // test multiple flag syntax
    {
        test_group_t group = test_group(&suite, "multiple flags");
        test_config_t cfg = make_config(arena);
        ap_t *ap = ap_new(cfg.root, arena);
        int result = ap_parse(ap, "-f a,c");

        GROUP_EXPECT_PASS(group, "smoke test parses", result == EXIT_OK);
        GROUP_EXPECT_PASS(group, "no unknown args", vector_len(ap_get_unknown(ap)) == 0);

        enum test_flags_t f = cfg_flags_value(cfg.flag_field);
        GROUP_EXPECT_PASS(group, "flag", f == (eTestFlagA | eTestFlagC));
    }

    // test flag negation
    {
        test_group_t group = test_group(&suite, "flag negation");
        test_config_t cfg = make_config(arena);
        ap_t *ap = ap_new(cfg.root, arena);
        int result = ap_parse(ap, "-f \"-a,c\"");

        GROUP_EXPECT_PASS(group, "smoke test parses", result == EXIT_OK);
        GROUP_EXPECT_PASS(group, "no unknown args", vector_len(ap_get_unknown(ap)) == 0);

        enum test_flags_t f = cfg_flags_value(cfg.flag_field);
        GROUP_EXPECT_PASS(group, "flag", f == eTestFlagC);
    }

    // test bool disable
    {
        test_group_t group = test_group(&suite, "bool disable");
        test_config_t cfg = make_config(arena);
        ap_t *ap = ap_new(cfg.root, arena);
        int result = ap_parse(ap, "/b-");

        GROUP_EXPECT_PASS(group, "smoke test parses", result == EXIT_OK);
        GROUP_EXPECT_PASS(group, "no unknown args", vector_len(ap_get_unknown(ap)) == 0);

        bool b = cfg_bool_value(cfg.bool_field);
        GROUP_EXPECT_PASS(group, "bool", b == false);
    }

    return test_suite_finish(&suite);
}
