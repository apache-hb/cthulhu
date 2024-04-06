// SPDX-License-Identifier: LGPL-3.0-or-later
#include "setup/setup2.h"

#include "arena/arena.h"
#include "argparse/argparse.h"
#include "backtrace/backtrace.h"
#include "base/log.h"
#include "base/panic.h"
#include "base/util.h"
#include "config/config.h"
#include "core/macros.h"
#include "format/backtrace.h"
#include "format/colour.h"
#include "format/config.h"
#include "format/version.h"
#include "io/console.h"
#include "io/io.h"
#include "memory/memory.h"
#include "notify/notify.h"
#include "os/core.h"
#include "setup/memory.h"
#include "std/str.h"
#include "std/vector.h"

#include <limits.h>

/// consts

#define NAME_BUFFER_SIZE 256
#define PATH_BUFFER_SIZE 512

/// the top bit is reserved for the exit flag
#define INT_BIT (sizeof(int) * CHAR_BIT)
#define SHOULD_EXIT (1 << (INT_BIT - 1))

static const cfg_info_t kGeneralGroupInfo = {
    .name = "general",
    .brief = "General options",
};

static const char *const kHelpInfoShortArgs[] = CT_ARGS("h", "?");
static const char *const kHelpInfoLongArgs[] = CT_ARGS("help");

static const cfg_info_t kHelpInfo = {
    .name = "help",
    .brief = "Print this help message and exit",
    .short_args = kHelpInfoShortArgs,
    .long_args = kHelpInfoLongArgs,
};

static const char *const kVersionInfoShortArgs[] = CT_ARGS("V");
static const char *const kVersionInfoLongArgs[] = CT_ARGS("version");

static const cfg_info_t kVersionInfo = {
    .name = "version",
    .brief = "Print version information and exit",
    .short_args = kVersionInfoShortArgs,
    .long_args = kVersionInfoLongArgs,
};

static const cfg_info_t kReportGroupInfo = {
    .name = "report",
    .brief = "Diangostic reporting options",
};

static const cfg_choice_t kHeadingOptions[] = {
    { .text = "generic", .value = eHeadingGeneric },
    { .text = "microsoft", .value = eHeadingMicrosoft },
};
#define HEADING_OPTION_COUNT (sizeof(kHeadingOptions) / sizeof(cfg_choice_t))

static const char *const kHeadingArgsShort[] = CT_ARGS("heading");

static const cfg_info_t kHeadingInfo = {
    .name = "heading",
    .brief = "Diagnostic heading style.",
    .short_args = kHeadingArgsShort,
};

static const char *const kColourInfoShortArgs[] = CT_ARGS("fcolour-diagnostics");

static const cfg_info_t kColourInfo = {
    .name = "colour",
    .brief = "Enable colour output",
    .short_args = kColourInfoShortArgs,
};

static const cfg_info_t kDebugGroupInfo = {
    .name = "debug",
    .brief = "Internal debugging options",
};

static const char *const kVerboseLoggingInfoShortArgs[] = CT_ARGS("v");
static const char *const kVerboseLoggingInfoLongArgs[] = CT_ARGS("verbose");

static const cfg_info_t kVerboseLoggingInfo = {
    .name = "verbose",
    .brief = "Enable verbose logging",
    .short_args = kVerboseLoggingInfoShortArgs,
    .long_args = kVerboseLoggingInfoLongArgs,
};

/// system error handler

static void default_error_begin(size_t error, void *user)
{
    io_printf(user, "a fatal error has occured 0x%zX\n", error);
}

static void default_error_next(bt_address_t frame, void *user)
{
    io_t *io = user;

    char name_buffer[NAME_BUFFER_SIZE];
    char path_buffer[PATH_BUFFER_SIZE];

    bt_symbol_t symbol = {
        .name = text_make(name_buffer, sizeof(name_buffer)),
        .path = text_make(path_buffer, sizeof(path_buffer))
    };

    frame_resolve_t resolve = bt_resolve_symbol(frame, &symbol);

    text_t name = symbol.name;
    text_t path = symbol.path;

    if (resolve & (eResolveLine | eResolveFile))
    {
        io_printf(io, "%s (%s:%" CT_PRI_LINE ")\n", name.text, path.text, symbol.line);
    }
    else
    {
        io_printf(io, "%s\n", name.text);
    }
}

static void default_error_end(void *user)
{
    CT_UNUSED(user);

    os_exit(CT_EXIT_INTERNAL);
}

/// panic handler

static void pretty_panic_handler(source_info_t location, const char *fmt, va_list args)
{
    arena_t *arena = get_global_arena();
    bt_report_t *report = bt_report_collect(arena);
    io_t *io = io_stderr();

    print_options_t print_options = {
        .arena = arena,
        .io = io,
        .pallete = &kColourDefault, // TODO: some of these should be globals
    };

    print_backtrace_t backtrace_config = {
        .options = print_options,
        .header = eHeadingGeneric,
        .zero_indexed_lines = false,
    };

    char *msg = str_vformat(arena, fmt, args);

    io_printf(io, "[panic][%s:%" CT_PRI_LINE "] => %s: %s\n", location.file, location.line, location.function, msg);

    print_backtrace(backtrace_config, report);
    os_exit(CT_EXIT_INTERNAL);
}

/// verbose callback

static void default_verbose(const char *fmt, va_list args)
{
    io_t *io = io_stdout();
    io_vprintf(io, fmt, args);
    io_printf(io, "\n");
}

/// public api

void setup_default(arena_t *arena)
{
    arena_t *mem = arena ? arena : ctu_default_alloc();
    bt_init();
    os_init();

    init_global_arena(mem);
    init_gmp_arena(mem);

    bt_error_t error = {
        .begin = default_error_begin,
        .end = default_error_end,
        .next = default_error_next,
        .user = io_stderr(),
    };

    gSystemError = error;
    gPanicHandler = pretty_panic_handler;
    gVerboseCallback = default_verbose;
}

setup_options_t setup_options(version_info_t info, cfg_group_t *root)
{
    CTASSERT(root != NULL);

    // general options

    cfg_group_t *general = config_group(root, &kGeneralGroupInfo);

    cfg_field_t *help = config_bool(general, &kHelpInfo, false);

    cfg_field_t *version = config_bool(general, &kVersionInfo, false);

    // report options

    cfg_group_t *report = config_group(general, &kReportGroupInfo);

    cfg_enum_t heading_info = {
        .options = kHeadingOptions,
        .count = HEADING_OPTION_COUNT,
        .initial = CT_DEFAULT_HEADER_STYLE,
    };

    cfg_field_t *header = config_enum(report, &kHeadingInfo, heading_info);

    cfg_field_t *colour = config_bool(report, &kColourInfo, false);

    // debug options

    cfg_group_t *debug = config_group(root, &kDebugGroupInfo);

    cfg_field_t *verbose = config_bool(debug, &kVerboseLoggingInfo, false);

    // argparse setup

    ap_t *ap = ap_new(root, ctu_default_alloc());

    setup_options_t options = {
        .version = info,
        .root = root,
        .ap = ap,

        .general = {
            .group = general,
            .help = help,
            .version = version,
        },
        .report = {
            .group = report,
            .header = header,
            .colour = colour,
        },
        .debug = {
            .group = debug,
            .verbose = verbose,
        }
    };

    return options;
}

/// setup parsing

static void report_ap_errors(io_t *io, format_context_t ctx, const vector_t *args)
{
    size_t len = vector_len(args);
    if (len == 0) return;

    char *err = colour_text(ctx, eColourRed, "error:");

    for (size_t i = 0; i < len; i++)
    {
        const char *arg = vector_get(args, i);
        io_printf(io, "%s: %s\n", err, arg);
    }
}

static void report_ap_unknown(io_t *io, format_context_t ctx, vector_t *args)
{
    size_t count = vector_len(args);
    if (count == 0) return;

    char *unk = colour_text(ctx, eColourYellow, "unknown argument");

    for (size_t i = 0; i < count; i++)
    {
        const char *arg = vector_get(args, i);
        io_printf(io, "%s: %s\n", unk, arg);
    }
}

static heading_style_t get_heading_style(const setup_options_t *setup)
{
    CTASSERT(setup != NULL);

    size_t style = cfg_enum_value(setup->report.header);
    return (style == eHeadingMicrosoft) ? eHeadingMicrosoft : eHeadingGeneric;
}

static setup_init_t setup_exit(int code)
{
    setup_init_t result = { 0 };
    result.exitcode = code | SHOULD_EXIT;
    return result;
}

static setup_init_t setup_help(
    const char *name,
    const colour_pallete_t *pallete,
    bool usage,
    heading_style_t style,
    const cfg_group_t *config)
{
    print_options_t base = {
        .arena = get_global_arena(),
        .io = io_stdout(),
        .pallete = pallete,
    };

    print_config_t print = {
        .options = base,
        .print_usage = usage,
        .win_style = (style == eHeadingMicrosoft),
        .name = name,
    };

    print_config(print, config);

    return setup_exit(CT_EXIT_OK);
}

static setup_init_t setup_version(
    const char *name,
    const colour_pallete_t *pallete,
    version_info_t version)
{
    print_options_t base = {
        .arena = get_global_arena(),
        .io = io_stdout(),
        .pallete = pallete,
    };

    print_version_t print = {
        .options = base,
    };

    print_version(print, version, name);

    return setup_exit(CT_EXIT_OK);
}

setup_init_t setup_parse(int argc, const char **argv, setup_options_t setup)
{
    CTASSERT(argc > 0);
    CTASSERT(argv != NULL);

    arena_t *arena = get_global_arena();
    io_t *io = io_stderr();

    int err = ap_parse_args(setup.ap, argc, argv);

    vector_t *errors = ap_get_errors(setup.ap);
    vector_t *unknown = ap_get_unknown(setup.ap);

    const colour_pallete_t *pallete = (cfg_bool_value(setup.report.colour) ? &kColourDefault : &kColourNone);
    heading_style_t heading = get_heading_style(&setup);

    format_context_t fmt = {
        .pallete = pallete,
        .arena = arena,
    };

    report_ap_errors(io, fmt, errors);
    report_ap_unknown(io, fmt, unknown);

    // if there was an error parsing the arguments, exit
    // errors have already been printed
    if (err != CT_EXIT_OK)
        return setup_exit(err);

    vector_t *posargs = ap_get_posargs(setup.ap);
    size_t pos = vector_len(posargs);
    size_t unknowns = vector_len(unknown);
    size_t params = ap_count_params(setup.ap);

    // no valid arguments were provided, print help and exit
    if (pos == 0 && (unknowns == 0 || params == 0))
        return setup_help(argv[0], pallete, false, heading, setup.root);

    // if the help flag was provided, print help and exit
    bool help = cfg_bool_value(setup.general.help);
    if (help)
        return setup_help(argv[0], pallete, true, heading, setup.root);

    // if the version flag was provided, print version and exit
    bool version = cfg_bool_value(setup.general.version);
    if (version)
        return setup_version(argv[0], pallete, setup.version);

    setup_init_t result = {
        .exitcode = CT_EXIT_OK,
        .posargs = posargs,
        .pallete = pallete,
        .heading = heading,
    };

    return result;
}

/// public accessor api

bool setup_should_exit(const setup_init_t *init)
{
    CTASSERT(init != NULL);

    return init->exitcode & SHOULD_EXIT;
}

int setup_exit_code(const setup_init_t *init)
{
    CTASSERT(setup_should_exit(init));

    return init->exitcode & ~SHOULD_EXIT;
}
