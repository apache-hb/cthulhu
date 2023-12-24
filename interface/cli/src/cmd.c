#include "cmd.h"

#include "config/config.h"
#include "support/langs.h"

#include "cthulhu/mediator/interface.h"

#include "core/macros.h"

#include "memory/memory.h"

#include "std/vector.h"
#include "std/str.h"

#include "notify/notify.h"

#include "argparse/argparse.h"
#include "argparse/commands.h"

#include <stdio.h>

// general
static const char *const kAddExtensionMapNames[] = { "-ext", "--add-ext", NULL };

// codegen
static const char *const kOutputSourceNames[] = { "-o", "--output", NULL };
static const char *const kOutputHeaderNames[] = { "-header", "--output-header", NULL };

// debug
static const char *const kDebugSsaNames[] = { "-dbgssa", "--debug-ssa", NULL };
static const char *const kDebugVerboseNames[] = { "-V", "--verbose", NULL };
static const char *const kWarnAsErrorNames[] = { "-Werror", NULL };
static const char *const kReportLimitNames[] = { "-fmax-errors", NULL };

// general events

static AP_EVENT(on_register_ext, ap, param, value, data)
{
    CTU_UNUSED(ap);
    CTU_UNUSED(param);

    runtime_t *rt = data;
    const char *mapping = value;

    size_t split = str_find(mapping, ":");
    if (split == SIZE_MAX)
    {
        printf("failed to register extension `%s` (invalid mapping)\n", mapping);
        printf("mappings take the form of `lang-id:ext`\n");
        return eEventHandled;
    }

    const char *id = ctu_strndup(mapping, split);
    const char *ext = ctu_strdup(mapping + split + 1);

    const language_t *lang = lifetime_get_language(rt->lifetime, id);
    if (lang == NULL)
    {
        printf("failed to register extension `%s` (no language identified by id=%s found)\n", mapping, id);
        return eEventHandled;
    }

    const language_t *old = lifetime_add_extension(rt->lifetime, ext, lang);
    if (old != NULL)
    {
        printf("failed to register extension `%s` (extension already registered by %s)\n", mapping, old->name);
        return eEventHandled;
    }

    return eEventHandled;
}

// codegen events

static AP_EVENT(on_set_source, ap, param, value, data)
{
    CTU_UNUSED(ap);
    CTU_UNUSED(param);

    const char *path = value;
    runtime_t *rt = data;

    rt->sourceOut = path;

    return eEventHandled;
}

static AP_EVENT(on_set_header, ap, param, value, data)
{
    CTU_UNUSED(ap);
    CTU_UNUSED(param);

    const char *path = value;
    runtime_t *rt = data;

    rt->headerOut = path;

    return eEventHandled;
}


// debug events

static AP_EVENT(on_set_debug_ssa, ap, param, value, data)
{
    CTU_UNUSED(ap);
    CTU_UNUSED(param);

    const bool *val = value;
    runtime_t *rt = data;

    rt->emitSSA = *val;

    return eEventHandled;
}

static AP_EVENT(on_set_verbose, ap, param, value, data)
{
    CTU_UNUSED(ap);
    CTU_UNUSED(param);
    CTU_UNUSED(data);
    CTU_UNUSED(value);

    return eEventHandled;
}

// posargs

static AP_EVENT(on_add_source, ap, param, value, data)
{
    CTU_UNUSED(ap);
    CTU_UNUSED(param);
    CTU_UNUSED(data);

    const char *path = value;
    runtime_t *rt = data;

    vector_push(&rt->sourcePaths, (char*)path);

    return eEventHandled;
}

// errors

const diagnostic_t kDiagUnknownArg = {
    .severity = eSeverityWarn,
    .id = "CLI-0001",
    .brief = "unknown argument",
    .description = "unknown argument provided to command line"
};

static AP_ERROR(on_arg_error, ap, node, message, data)
{
    CTU_UNUSED(ap);

    runtime_t *rt = data;

    msg_notify(rt->reports, &kDiagUnknownArg, node, "%s", message);

    return eEventHandled;
}

static const cfg_info_t kConfigInfo = {
    .name = "cli",
    .brief = "Cthulhu command line interface",
    .description = "Cthulhu CLI configuration options",
};

/// general
// static cfg_info_t kGroup_GeneralInfo = {
//     .name = "general",
//     .brief = "General options"
// };

// static cfg_info_t kGeneral_HelpInfo = {
//     .name = "help",
//     .brief = "Display help information",
//     .description = "Display help information"
// };

// static cfg_info_t kGeneral_VersionInfo = {
//     .name = "version",
//     .brief = "Display version information",
//     .description = "Display version information"
// };

// /// codegen

// static cfg_info_t kGroup_CodegenInfo = {
//     .name = "codegen",
//     .brief = "Code generation options"
// };

// /// compiler debugging, user debugging options should be in codegen

// static cfg_info_t kGroup_DebugInfo = {
//     .name = "debug",
//     .brief = "Debugging options",
//     .description = "Compiler internal debugging options, for user debugging options see codegen"
// };

// /// reporting

// static cfg_info_t kGroup_ReportInfo = {
//     .name = "reports",
//     .brief = "Reporting options"
// };

runtime_t cmd_parse(mediator_t *mediator, lifetime_t *lifetime, int argc, const char **argv)
{
    arena_t *arena = lifetime_get_arena(lifetime);
    langs_t langs = get_langs();
    for (size_t i = 0; i < langs.size; i++)
    {
        lifetime_add_language(lifetime, langs.langs + i);
    }

    config_t *config = config_new(arena, &kConfigInfo);

    ap_t *ap = ap_new("cli", NEW_VERSION(0, 0, 1), config, arena);

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


    // config_t *config = config_new(arena, &kConfigInfo);
    // config_t *general_group = config_group(config, &kGroup_GeneralInfo);

    // // general
    // cfg_bool_t help_options = {.initial = false};
    // cfg_field_t *help_field = config_bool(general_group, &kGeneral_HelpInfo, help_options);

    // cfg_bool_t version_options = {.initial = false};
    // cfg_field_t *version_field = config_bool(general_group, &kGeneral_VersionInfo, version_options);

    // // codegen
    // config_t *codegen_group = config_group(config, &kGroup_CodegenInfo);

    // // debug
    // config_t *debug_group = config_group(config, &kGroup_DebugInfo);

    // // reporting
    // config_t *report_group = config_group(config, &kGroup_ReportInfo);


    ap_group_t *generalGroup = ap_group_new(ap, "general", "general options");
    ap_param_t *addExtensionMapParam = ap_add_string(generalGroup, "add extension map", "register a new extension for a compiler", kAddExtensionMapNames);

    ap_group_t *codegenGroup = ap_group_new(ap, "codegen", "code generation options");
    ap_param_t *outputFileParam = ap_add_string(codegenGroup, "output file name", "output file name, will have .c appened to it (default: out)", kOutputSourceNames);
    ap_param_t *outputHeaderParam = ap_add_string(codegenGroup, "output header", "output header name, provide none to skip header generation. will have .c appeneded to it (default: none)", kOutputHeaderNames);

    ap_group_t *debugGroup = ap_group_new(ap, "reports", "reporting options");
    ap_param_t *debugSsaParam = ap_add_bool(debugGroup, "debug ssa", "print debug ssa output", kDebugSsaNames);
    ap_param_t *debugVerboseParam = ap_add_bool(debugGroup, "verbose", "enable verbose logging", kDebugVerboseNames);
    ap_param_t *warnAsErrorParam = ap_add_bool(debugGroup, "warn as error", "treat warnings as errors", kWarnAsErrorNames);
    ap_param_t *reportLimitParam = ap_add_int(debugGroup, "report limit", "limit the number of reports to display (default: 20)", kReportLimitNames);

    CTU_UNUSED(warnAsErrorParam);
    CTU_UNUSED(reportLimitParam);

    // general
    ap_event(ap, addExtensionMapParam, on_register_ext, &rt);

    // debug
    ap_event(ap, debugSsaParam, on_set_debug_ssa, &rt);
    ap_event(ap, debugVerboseParam, on_set_verbose, &rt);

    ap_event(ap, outputFileParam, on_set_source, &rt);
    ap_event(ap, outputHeaderParam, on_set_header, &rt);

    ap_event(ap, NULL, on_add_source, &rt);
    ap_error(ap, on_arg_error, &rt);

    ap_parse(ap, argc, argv);

    return rt;
}
