#include "cmd.h"

#include "base/log.h"
#include "config/config.h"
#include "io/console.h"
#include "io/io.h"
#include "support/langs.h"

#include "cthulhu/runtime/interface.h"

#include "std/vector.h"
#include "std/str.h"

#include "notify/notify.h"

#include "argparse/argparse.h"

#include <stdio.h>

// errors

const diagnostic_t kDiagUnknownArg = {
    .severity = eSeverityWarn,
    .id = "CLI-0001",
    .brief = "unknown argument",
    .description = "unknown argument provided to command line"
};

static const cfg_info_t kConfigInfo = {
    .name = "cli",
    .brief = "Cthulhu CLI configuration options",
};

/// general
static cfg_info_t kGroup_GeneralInfo = {
    .name = "general",
    .brief = "General options"
};

static const char *const kGeneralHelpInfoShortArgs[] = { "h", "?", NULL };
static const char *const kGeneralHelpInfoLongArgs[] = { "help", NULL };

static cfg_info_t kGeneral_HelpInfo = {
    .name = "help",
    .brief = "Print help message",
    .short_args = kGeneralHelpInfoShortArgs,
    .long_args = kGeneralHelpInfoLongArgs,
};

static const char *const kGeneralVersionInfoShortArgs[] = { "V", NULL };
static const char *const kGeneralVersionInfoLongArgs[] = { "version", NULL };

static cfg_info_t kGeneral_VersionInfo = {
    .name = "version",
    .brief = "Print version message",
    .short_args = kGeneralVersionInfoShortArgs,
    .long_args = kGeneralVersionInfoLongArgs,
};

/// codegen

static cfg_info_t kGroup_CodegenInfo = {
    .name = "codegen",
    .brief = "Code generation options"
};

static const char *const kCodegenEmitSsaShortArgs[] = { "ssa", NULL };
static const char *const kCodegenEmitSsaLongArgs[] = { "emit-ssa", NULL };

static cfg_info_t kCodegen_EmitSsa = {
    .name = "debug-ssa",
    .brief = "Emit SSA to the output directory",
    .short_args = kCodegenEmitSsaShortArgs,
    .long_args = kCodegenEmitSsaLongArgs,
};

/// compiler debugging, user debugging options should be in codegen

static cfg_info_t kGroup_DebugInfo = {
    .name = "debug",
    .brief = "Compiler internal debugging options, for user debugging options see codegen",
};

static const char *const kDebugVerboseLogsShortArgs[] = { "V", NULL };
static const char *const kDebugVerboseLogsLongArgs[] = { "verbose", NULL };

static cfg_info_t kDebug_VerboseLogs = {
    .name = "verbose",
    .brief = "Enable verbose logging",
    .short_args = kDebugVerboseLogsShortArgs,
    .long_args = kDebugVerboseLogsLongArgs,
};

/// reporting

static cfg_info_t kGroup_ReportInfo = {
    .name = "reports",
    .brief = "Reporting options"
};

static const char *const kReportWarnAsErrorArgs[] = { "Werror", NULL };

static cfg_info_t kReport_WarnAsError = {
    .name = "warn-as-error",
    .brief = "Treat warnings as errors",
    .short_args = kReportWarnAsErrorArgs,
    .long_args = kReportWarnAsErrorArgs,
};

static const char *const kReportLimitArgs[] = { "fmax-errors", NULL };

static cfg_info_t kReport_Limit = {
    .name = "max-errors",
    .brief = "Limit the number of reports",
    .short_args = kReportLimitArgs,
};

runtime_t cmd_parse(mediator_t *mediator, lifetime_t *lifetime, int argc, const char **argv)
{
    arena_t *arena = lifetime_get_arena(lifetime);
    io_t *io = io_stdout(arena);
    langs_t langs = get_langs();
    for (size_t i = 0; i < langs.size; i++)
    {
        lifetime_add_language(lifetime, langs.langs[i]);
    }

    config_t *config = config_new(arena, &kConfigInfo);

    // general
    config_t *general_group = config_group(config, &kGroup_GeneralInfo);

    cfg_bool_t help_options = {.initial = false};
    cfg_field_t *help_field = config_bool(general_group, &kGeneral_HelpInfo, help_options);

    cfg_bool_t version_options = {.initial = false};
    cfg_field_t *version_field = config_bool(general_group, &kGeneral_VersionInfo, version_options);

    // codegen
    config_t *codegen_group = config_group(config, &kGroup_CodegenInfo);

    cfg_bool_t emit_ssa_options = {.initial = false};
    cfg_field_t *emit_ssa_field = config_bool(codegen_group, &kCodegen_EmitSsa, emit_ssa_options);

    // debug
    config_t *debug_group = config_group(config, &kGroup_DebugInfo);

    cfg_bool_t verbose_options = {.initial = false};
    cfg_field_t *verbose_field = config_bool(debug_group, &kDebug_VerboseLogs, verbose_options);

    // reporting
    config_t *report_group = config_group(config, &kGroup_ReportInfo);

    cfg_int_t report_limit_options = {.initial = 20, .min = 0, .max = 1000};
    cfg_field_t *report_limit_field = config_int(report_group, &kReport_Limit, report_limit_options);

    cfg_bool_t warn_as_error_options = {.initial = false};
    cfg_field_t *warn_as_error_field = config_bool(report_group, &kReport_WarnAsError, warn_as_error_options);

    ap_t *ap = ap_new(config, arena);

    runtime_t rt = {
        .argc = argc,
        .argv = argv,

        .mediator = mediator,
        .lifetime = lifetime,

        .reports = lifetime_get_logger(lifetime),
        .ap = ap,

        .warnAsError = false,
        .reportLimit = 20,

        .sourcePaths = vector_new(16),
    };

    ap_parse(ap, argc, argv);

    vector_t *posargs = ap_get_posargs(ap);
    size_t posarg_count = vector_len(posargs);

    for (size_t i = 0; i < posarg_count; i++)
    {
        const char *path = vector_get(posargs, i);
        vector_push(&rt.sourcePaths, (char*)path);
    }

    vector_t *unknown = ap_get_unknown(ap);
    size_t unknown_count = vector_len(unknown);

    if (unknown_count > 0)
    {
        io_printf(io, "%zu unknown arguments:\n", unknown_count);
        for (size_t i = 0; i < unknown_count; i++)
        {
            const char *arg = vector_get(unknown, i);
            io_printf(io, "  %s\n", arg);
        }
    }

    if (cfg_bool_value(verbose_field))
    {
        ctu_log_control(eLogEnable);
    }

    if (cfg_bool_value(help_field))
    {
        // print help
    }

    if (cfg_bool_value(version_field))
    {
        // print version
    }

    rt.reportLimit = cfg_int_value(report_limit_field);
    rt.warnAsError = cfg_bool_value(warn_as_error_field);

    rt.emitSSA = cfg_bool_value(emit_ssa_field);

    return rt;
}
