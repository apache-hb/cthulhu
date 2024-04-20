// SPDX-License-Identifier: GPL-3.0-only

#include "argparse/argparse.h"
#include "format/backtrace.h"
#include "format/colour.h"
#include "config/config.h"
#include "core/macros.h"
#include "setup/setup.h"
#include "format/config.h"
#include "format/notify2.h"
#include "io/console.h"
#include "io/io.h"

#include "memory/memory.h"

#include "notify/notify.h"
#include "format/notify.h"

#include "scan/node.h"

#include <tool_config.h>

typedef struct tool_t
{
    cfg_group_t *m_config;

    cfg_field_t *test_backtrace;
    cfg_field_t *notify_style;
    cfg_field_t *heading_style;
    cfg_field_t *zero_indexed;

    setup_options_t options;
} tool_t;

static const version_info_t kToolVersion = {
    .license = "GPLv3",
    .desc = "Notification testing tool",
    .author = "Elliot Haisley",
    .version = CT_NEW_VERSION(0, 0, 1),
};

static const cfg_info_t kToolInfo = {
    .name = "notify",
    .brief = "Notification testing options",
};

static const cfg_arg_t kBacktraceArgs[] = { CT_ARG_SHORT("bt"), CT_ARG_LONG("backtrace") };

static const cfg_info_t kBacktraceInfo = {
    .name = "backtrace",
    .brief = "Print a backtrace",
    .args = CT_ARGS(kBacktraceArgs),
};

static const cfg_choice_t kNotifyOptions[] = {
    { .text = "brief", .value = eNotifyBrief },
    { .text = "full", .value = eNotifyFull },
    { .text = "none", .value = eNotifyCount },
};
#define NOTIFY_OPTION_COUNT (sizeof(kNotifyOptions) / sizeof(cfg_choice_t))

static const cfg_arg_t kNotifyArgs[] = { CT_ARG_SHORT("n"), CT_ARG_LONG("notify") };

static const cfg_info_t kNotifyInfo = {
    .name = "notify",
    .brief = "Notification style",
    .args = CT_ARGS(kNotifyArgs),
};

static const cfg_choice_t kHeadingOptions[] = {
    { .text = "generic", .value = eHeadingGeneric },
    { .text = "microsoft", .value = eHeadingMicrosoft },
};
#define HEADING_OPTION_COUNT (sizeof(kHeadingOptions) / sizeof(cfg_choice_t))

static const cfg_arg_t kHeadingArgs[] = { CT_ARG_LONG("heading") };

static const cfg_info_t kHeadingInfo = {
    .name = "heading",
    .brief = "Heading style",
    .args = CT_ARGS(kHeadingArgs),
};

static const cfg_arg_t kZeroIndexedArgs[] = { CT_ARG_SHORT("zi"), CT_ARG_LONG("zero-indexed") };

static const cfg_info_t kZeroIndexedInfo = {
    .name = "zero_indexed",
    .brief = "Print zero indexed line numbers",
    .args = CT_ARGS(kZeroIndexedArgs),
};

static tool_t make_config(arena_t *arena)
{
    cfg_group_t *config = config_root(&kToolInfo, arena);

    setup_options_t defaults = setup_options(kToolVersion, config);

    cfg_field_t *test_backtrace = config_bool(config, &kBacktraceInfo, false);
    cfg_enum_t notify_info = {
        .options = kNotifyOptions,
        .count = NOTIFY_OPTION_COUNT,
        .initial = eNotifyCount,
    };

    cfg_field_t *notify_style = config_enum(config, &kNotifyInfo, notify_info);

    cfg_enum_t heading_info = {
        .options = kHeadingOptions,
        .count = HEADING_OPTION_COUNT,
        .initial = CT_DEFAULT_HEADER_STYLE,
    };

    cfg_field_t *heading = config_enum(config, &kHeadingInfo, heading_info);

    cfg_field_t *zero_indexed = config_bool(config, &kZeroIndexedInfo, false);

    tool_t tool = {
        .m_config = config,
        .test_backtrace = test_backtrace,
        .notify_style = notify_style,
        .heading_style = heading,
        .zero_indexed = zero_indexed,

        .options = defaults,
    };

    return tool;
}

const char *const kSampleSourceLeft =
    "module multi.lhs;\n"   // 1
    "\n"                    // 2
    "import multi.rhs,\n"   // 3
    "       main;\n"        // 4
    "\n"                    // 5
    "procedure lhs;\n"      // 3
    "begin\n"               // 4
    "    x := x + 1;\n"     // 5
    "    if x < LIMIT then\n" // 6
    "        call rhs\n"    // 7
    "end;\n"                // 8
    ".\n";                  // 9

const char *const kSampleSourceRight =
    "module multi.rhs;\n"   // 1
    "\n"                    // 2
    "import multi.lhs,\n"   // 3
    "       main;\n"        // 4
    "\n"                    // 5
    "procedure rhs;\n"      // 6
    "begin\n"               // 7
    "    x := x + 1;\n"     // 8
    "    if x < LIMIT then\n" // 9
    "        call lhs\n"    // 10
    "end;\n"                // 11
    ".\n";                  // 12

const char *const kSampleSourceMain =
    "module main;\n"        // 1
    "\n"                    // 2
    "import multi.lhs,\n"   // 3
    "       multi.rhs;\n"   // 4
    "\n"                    // 5
    "const LIMIT = 25;\n"   // 6
    "var x;\n"              // 7
    "\n"                    // 8
    "procedure entry;\n"    // 9
    "begin\n"               // 10
    "    x := 0;\n"         // 11
    "    call lhs;\n"       // 12
    "    ! x\n"             // 13
    "end;\n"                // 14
    "\n"                    // 15
    "call entry\n"          // 16
    ".\n";                  // 17

static const diagnostic_t kInfoDiagnostic = {
    .severity = eSeverityInfo,
    .id = "T0001",
    .brief = "Test diagnostic at info level",
    .description =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"
        "Sed non risus. Suspendisse lectus tortor, dignissim sit amet,\n"
        "adipiscing nec, ultricies sed, dolor. Cras elementum ultrices diam.\n"
        "Maecenas ligula massa, varius a, semper congue, euismod non, mi.\n"
        "Proin porttitor, orci nec nonummy molestie, enim est eleifend mi,\n"
        "non fermentum diam nisl sit amet erat. Duis semper. Duis arcu massa,\n"
        "scelerisque vitae, consequat in, pretium a, enim. Pellentesque congue.\n"
};

static const diagnostic_t kUndefinedFunctionName = {
    .severity = eSeverityFatal,
    .id = "G0001",
    .brief = "Undefined function name",
    .description =
        "A function name must be visible in the current scope\n"
        "to be used in a call expression or statement.\n"
};

static const diagnostic_t kUnresolvedImport = {
    .severity = eSeverityFatal,
    .id = "G0002",
    .brief = "Unresolved import",
    .description =
        "An import statement must refer to a valid module.\n"
        "The module must be visible in the current scope.\n"
};

static const diagnostic_t kReservedName = {
    .severity = eSeverityFatal,
    .id = "G0003",
    .brief = "Reserved name",
    .description =
        "A reserved name cannot be used as an identifier.\n"
        "Reserved names are keywords and builtin names.\n"
};

void event_simple(logger_t *logs, const node_t *builtin)
{
    event_builder_t event = msg_notify(logs, &kInfoDiagnostic, builtin, "test");
    msg_append(event, builtin, "hello %s", "world");
}

void event_missing_call(logger_t *logs, scan_t *scan_main, scan_t *scan_lhs, const node_t *builtin)
{
    where_t where = {
        .first_line = 11,
        .last_line = 11,
        .first_column = 4,
        .last_column = 4 + 8
    };

    node_t *node = node_new(scan_main, where);

    where_t where2 = {
        .first_line = 8,
        .last_line = 8,
        .first_column = 4,
        .last_column = 4 + 8
    };

    node_t *node2 = node_new(scan_lhs, where2);

    where_t where3 = {
        .first_line = 12,
        .last_line = 12,
        .first_column = 8,
        .last_column = 8 + 3
    };

    node_t *node3 = node_new(scan_lhs, where3);

    event_builder_t event = msg_notify(logs, &kUndefinedFunctionName, node, "function `%s` is undefined in the current context", "lhs");
    msg_note(event, "did you mean `%s`?", "rhs");
    msg_append(event, node, "function called here");
    msg_append(event, node, "function called here but with a different message");
    msg_append(event, node2, "function defined here");
    msg_append(event, node3, "foo bar");

    msg_append(event, builtin, "builtin node");
}

void event_invalid_import(logger_t *logs, scan_t *scan, scan_t *scan_rhs)
{
    where_t where = {
        .first_line = 2,
        .last_line = 2,
        .first_column = 7,
        .last_column = 7 + 9
    };

    node_t *node = node_new(scan, where);

    where_t where2 = {
        .first_line = 3,
        .last_line = 3,
        .first_column = 7,
        .last_column = 7 + 9
    };

    node_t *node2 = node_new(scan_rhs, where2);

    event_builder_t event = msg_notify(logs, &kUnresolvedImport, node, "unresolved import `%s`", "multi.lhs");
    msg_note(event, "did you mean `%s`?", "multi.rhs");
    msg_note(event, "did you mean `%s`?", "multi.rhx");
    msg_append(event, node, "import statement here");
    msg_append(event, node2, "module declaration here");
}

void event_invalid_function(logger_t *logs, scan_t *scan)
{
    where_t where = {
        .first_line = 8,
        .last_line = 13,
        .first_column = 0,
        .last_column = 4
    };

    node_t *node = node_new(scan, where);

    event_builder_t event = msg_notify(logs, &kReservedName, node, "reserved name `%s`", "entry");
    msg_append(event, node, "procedure declaration here");
    msg_note(event, "did you mean `%s`?", "main");
}

static scan_t *scan_string(const char *name, const char *lang, const char *source, arena_t *arena)
{
    io_t *io = io_string(name, source, arena);
    return scan_io(lang, io, arena);
}

static void do_print_backtrace(print_backtrace_t config, arena_t *arena)
{
    bt_report_t *report = bt_report_collect(arena);

    print_backtrace(config, report);
}

int recurse(int x, print_backtrace_t config, arena_t *arena)
{
    if (x == 0)
    {
        do_print_backtrace(config, arena);
        return 0;
    }

    return recurse(x - 1, config, arena);
}

static int rec3(int x, print_backtrace_t config, arena_t *arena)
{
    if (x == 0)
    {
        do_print_backtrace(config, arena);
        return 0;
    }

    return recurse(x - 1, config, arena);
}

static int inner(int x, print_backtrace_t config, arena_t *arena)
{
    return rec3(x, config, arena);
}

static int rec2(int x, int y, print_backtrace_t config, arena_t *arena)
{
    if (x == 0)
    {
        return inner(y, config, arena);
    }

    return rec2(x - 1, y, config, arena);
}

static int recurse_head(int x, print_backtrace_t config, arena_t *arena);
static int recurse_middle(int x, print_backtrace_t config, arena_t *arena);
static int recurse_tail(int x, print_backtrace_t config, arena_t *arena);

static int recurse_head(int x, print_backtrace_t config, arena_t *arena)
{
    if (x == 0)
    {
        do_print_backtrace(config, arena);
        return 0;
    }

    return recurse_middle(x - 1, config, arena);
}

static int recurse_middle(int x, print_backtrace_t config, arena_t *arena)
{
    if (x == 0)
    {
        do_print_backtrace(config, arena);
        return 0;
    }

    return recurse_tail(x - 1, config, arena);
}

static int recurse_tail(int x, print_backtrace_t config, arena_t *arena)
{
    if (x == 0)
    {
        do_print_backtrace(config, arena);
        return 0;
    }

    return recurse_head(x - 1, config, arena);
}

static void do_backtrace(io_t *io, arena_t *arena)
{
    io_printf(io, "\n=== backtrace ===\n\n");

    const char *source_root = TOOL_ROOT;

    io_printf(io, "source root: %s\n\n", source_root);

    print_options_t options = {
        .arena = arena,
        .io = io,
        .pallete = &kColourDefault,
    };

    print_backtrace_t config1 = {
        .options = options,
        .header = eHeadingGeneric,
        .zero_indexed_lines = false,
        .project_source_path = source_root,
    };

    print_backtrace_t config2 = {
        .options = options,
        .header = eHeadingMicrosoft,
        .zero_indexed_lines = true,
        .project_source_path = source_root,
    };

    recurse(15, config1, arena);
    recurse(1000, config2, arena);

    rec2(200, 100, config1, arena);
    rec2(5, 100, config2, arena);

    recurse_head(25, config1, arena);
}

int main(int argc, const char **argv)
{
    setup_default(NULL);

    arena_t *arena = get_global_arena();
    io_t *con = io_stdout();
    tool_t tool = make_config(arena);
    node_t *node = node_builtin("notify", arena);

    setup_init_t setup = setup_parse(argc, argv, tool.options);

    if (setup_should_exit(&setup))
        return setup_exit_code(&setup);

    bool backtraces = cfg_bool_value(tool.test_backtrace);

    notify_style_t style = cfg_enum_value(tool.notify_style);
    heading_style_t heading = cfg_enum_value(tool.heading_style);
    bool zero_indexed = cfg_bool_value(tool.zero_indexed);

    logger_t *logs = logger_new(arena);

    scan_t *scan_main = scan_string("sample.pl0", "PL/0", kSampleSourceMain, arena);
    scan_t *scan_lhs = scan_string("lhs.mod", "Oberon-2", kSampleSourceLeft, arena);
    scan_t *scan_rhs = scan_string("rhs.ctu", "Cthulhu", kSampleSourceRight, arena);

    event_simple(logs, node);
    event_missing_call(logs, scan_main, scan_lhs, node);
    event_invalid_import(logs, scan_main, scan_rhs);
    event_invalid_function(logs, scan_main);

    if (style != eNotifyCount)
    {
        print_options_t options = {
            .arena = arena,
            .io = con,
            .pallete = setup.pallete
        };

        print_notify_t notify_options = {
            .options = options,
            .heading = heading,
            .style = style,
            .zero_indexed_lines = zero_indexed,
        };

        const typevec_t *events = logger_get_events(logs);
        print_notify_many(notify_options, events);
    }

    if (backtraces)
    {
        do_backtrace(con, arena);
    }

    io_close(con);
}
