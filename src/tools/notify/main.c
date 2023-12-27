#include "argparse/argparse.h"
#include "base/colour.h"
#include "config/config.h"
#include "core/macros.h"
#include "defaults/defaults.h"
#include "display/display.h"
#include "io/console.h"
#include "io/io.h"

#include "memory/memory.h"

#include "notify/notify.h"
#include "notify/text.h"

#include "scan/node.h"

typedef struct tool_t
{
    config_t *config;

    cfg_field_t *test_backtrace;
    cfg_field_t *test_simple;
    cfg_field_t *test_rich;

    default_options_t options;
} tool_t;

static const version_info_t kToolVersion = {
    .license = "GPLv3",
    .desc = "Notification testing tool",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1),
};

static const cfg_info_t kToolInfo = {
    .name = "notify",
    .brief = "Notification testing options",
};

static const char *const kBacktraceArgsShort[] = { "bt", NULL };
static const char *const kBacktraceArgsLong[] = { "backtrace", NULL };

static const cfg_info_t kBacktraceInfo = {
    .name = "backtrace",
    .brief = "Print a backtrace",
    .short_args = kBacktraceArgsShort,
    .long_args = kBacktraceArgsLong,
};

static const char *const kSimpleArgsShort[] = { "simple", NULL };

static const cfg_info_t kSimpleInfo = {
    .name = "simple",
    .brief = "Print a simple report",
    .short_args = kSimpleArgsShort,
};

static const char *const kRichArgsShort[] = { "rich", NULL };

static const cfg_info_t kRichInfo = {
    .name = "rich",
    .brief = "Print a rich report",
    .short_args = kRichArgsShort,
};

static tool_t make_config(arena_t *arena)
{
    config_t *config = config_new(arena, &kToolInfo);

    cfg_bool_t initial = { .initial = false };
    cfg_field_t *test_backtrace = config_bool(config, &kBacktraceInfo, initial);
    cfg_field_t *test_simple = config_bool(config, &kSimpleInfo, initial);
    cfg_field_t *test_rich = config_bool(config, &kRichInfo, initial);

    default_options_t defaults = get_default_options(config);

    tool_t tool = {
        .config = config,
        .test_backtrace = test_backtrace,
        .test_simple = test_simple,
        .test_rich = test_rich,
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

void event_simple(logger_t *logs)
{
    event_t *event = msg_notify(logs, &kInfoDiagnostic, node_builtin(), "test");
    msg_append(event, node_builtin(), "hello %s", "world");
}

void event_missing_call(logger_t *logs, scan_t *scan_main, scan_t *scan_lhs)
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

    event_t *event = msg_notify(logs, &kUndefinedFunctionName, node, "function `%s` is undefined in the current context", "lhs");
    msg_note(event, "did you mean `%s`?", "rhs");
    msg_append(event, node, "function called here");
    msg_append(event, node, "function called here but with a different message");
    msg_append(event, node2, "function defined here");
    msg_append(event, node3, "foo bar");

    msg_append(event, node_builtin(), "builtin node");
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

    event_t *event = msg_notify(logs, &kUnresolvedImport, node, "unresolved import `%s`", "multi.lhs");
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

    event_t *event = msg_notify(logs, &kReservedName, node, "reserved name `%s`", "entry");
    msg_append(event, node, "procedure declaration here");
    msg_note(event, "did you mean `%s`?", "main");
}

static scan_t *scan_string(const char *name, const char *lang, const char *source, arena_t *arena)
{
    io_t *io = io_string(name, source, arena);
    return scan_io(lang, io, arena);
}

static void print_backtrace(text_config_t base_config)
{
    text_config_t config = base_config;
    arena_t *arena = ctu_default_alloc();
    config.io = io_stdout(arena);

    bt_report_t report = bt_report_collect(arena);

    bt_report_finish(config, &report);
}

int recurse(int x, text_config_t base_config)
{
    if (x == 0)
    {
        print_backtrace(base_config);
        return 0;
    }

    return recurse(x - 1, base_config);
}

static int rec3(int x, text_config_t base_config)
{
    if (x == 0)
    {
        print_backtrace(base_config);
        return 0;
    }

    return recurse(x - 1, base_config);
}

static int inner(int x, text_config_t base_config)
{
    return rec3(x, base_config);
}

static int rec2(int x, int y, text_config_t base_config)
{
    if (x == 0)
    {
        return inner(y, base_config);
    }

    return rec2(x - 1, y, base_config);
}

static void do_backtrace(io_t *io)
{
    io_printf(io, "\n=== backtrace ===\n\n");

    text_config_t bt_config2 = {
        .config = {
            .zeroth_line = false,
            .print_source = true,
            .print_header = true
        },
        .colours = &kColourDefault,
        .io = io
    };

    text_config_t bt_config1 = {
        .config = {
            .zeroth_line = false,
            .print_source = false,
            .print_header = true
        },
        .colours = &kColourDefault,
        .io = io
    };

    recurse(15, bt_config1);
    recurse(1000, bt_config2);

    rec2(200, 100, bt_config1);
    rec2(5, 100, bt_config2);
}

static void do_simple(logger_t *logs)
{
    io_t *io = io_stdout(ctu_default_alloc());

    text_config_t config2 = {
        .config = {
            .zeroth_line = false,
        },
        .colours = &kColourDefault,
        .io = io
    };

    report_config_t report_config = {
        .max_errors = SIZE_MAX,
        .max_warnings = SIZE_MAX,
        .report_format = eTextSimple,

        .text_config = config2,
    };

    typevec_t *events = logger_get_events(logs);

    text_report(events, report_config, "simple text");
}

static void do_rich(logger_t *logs)
{
    text_config_t config = {
        .config = {
            .zeroth_line = false,
        },
        .colours = &kColourDefault,
        .io = io_stdout(ctu_default_alloc())
    };

    report_config_t report_config = {
        .max_errors = SIZE_MAX,
        .max_warnings = SIZE_MAX,
        .report_format = eTextComplex,

        .text_config = config,
    };

    typevec_t *events = logger_get_events(logs);

    text_report(events, report_config, "rich text");
}

int main(int argc, const char **argv)
{
    default_init();

    arena_t *arena = get_global_arena();
    io_t *io = io_stdout(arena);
    tool_t tool = make_config(arena);

    tool_config_t config = {
        .arena = arena,
        .io = io,

        .group = tool.config,
        .version = kToolVersion,

        .argc = argc,
        .argv = argv,
    };

    int err = parse_commands(tool.options, config);
    if (err == EXIT_SHOULD_EXIT)
    {
        return EXIT_OK;
    }

    bool backtraces = cfg_bool_value(tool.test_backtrace);
    bool simple = cfg_bool_value(tool.test_simple);
    bool rich = cfg_bool_value(tool.test_rich);

    logger_t *logs = logger_new(arena);

    scan_t *scan_main = scan_string("sample.pl0", "PL/0", kSampleSourceMain, arena);
    scan_t *scan_lhs = scan_string("lhs.mod", "Oberon-2", kSampleSourceLeft, arena);
    scan_t *scan_rhs = scan_string("rhs.ctu", "Cthulhu", kSampleSourceRight, arena);

    event_simple(logs);
    event_missing_call(logs, scan_main, scan_lhs);
    event_invalid_import(logs, scan_main, scan_rhs);
    event_invalid_function(logs, scan_main);

    if (!rich && !simple && !backtraces)
    {
        rich = true;
        simple = true;
        backtraces = true;
    }

    if (rich)
        do_rich(logs);

    if (simple)
        do_simple(logs);

    if (backtraces)
        do_backtrace(io);

    io_close(io);
}
