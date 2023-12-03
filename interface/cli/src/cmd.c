#include "cmd.h"

#include "support/langs.h"

#include "cthulhu/mediator/interface.h"

#include "base/util.h"
#include "core/macros.h"

#include "io/io.h"

#include "std/vector.h"
#include "std/str.h"

#include "report/report.h"

#include "argparse/argparse.h"
#include "argparse/commands.h"

#include <stdio.h>

// general
static const char *kHelpNames[] = { "-h", "--help", NULL };
static const char *kVersionNames[] = { "-v", "--version", NULL };
static const char *kAddExtensionMapNames[] = { "-ext", "--add-ext", NULL };

// codegen
static const char *kOutputSourceNames[] = { "-o", "--output", NULL };
static const char *kOutputHeaderNames[] = { "-header", "--output-header", NULL };

// debug
static const char *kDebugSsaNames[] = { "-dbgssa", "--debug-ssa", NULL };
static const char *kDebugVerboseNames[] = { "-V", "--verbose", NULL };
static const char *kWarnAsErrorNames[] = { "-Werror", NULL };
static const char *kReportLimitNames[] = { "-fmax-errors", NULL };

// general events

static AP_EVENT(on_help, ap, param, value, data)
{
    CTU_UNUSED(param);
    CTU_UNUSED(value);

    runtime_t *rt = data;
    ap_print_help_header(ap, rt->argv[0]);

    langs_t langs = get_langs();


    printf("\n%zu languages loaded:\n", langs.size);
    printf("========================================\n");

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        ap_print_version_info(lang->version, lang->name);
        printf("========================================\n");
    }

    printf("\n");


    ap_print_help_body(ap, rt->argv[0]);

    return eEventHandled;
}

static AP_EVENT(on_version, ap, param, value, data)
{
    CTU_UNUSED(param);
    CTU_UNUSED(value);
    CTU_UNUSED(data);

    ap_version(ap);

    return eEventHandled;
}

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

    logverbose("registered extension `%s` to language `%s`", ext, lang->id);

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

    logverbose("%sd debug ssa", *val ? "enable" : "disable");

    return eEventHandled;
}

static AP_EVENT(on_set_verbose, ap, param, value, data)
{
    CTU_UNUSED(ap);
    CTU_UNUSED(param);
    CTU_UNUSED(data);

    const bool *val = value;

    verbose = *val;

    logverbose("%sd verbose logging", verbose ? "enable" : "disable");

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

static AP_ERROR(on_arg_error, ap, node, message, data)
{
    CTU_UNUSED(ap);

    runtime_t *rt = data;

    report(rt->reports, eFatal, node, "%s", message);

    return eEventHandled;
}

runtime_t cmd_parse(reports_t *reports, mediator_t *mediator, lifetime_t *lifetime, int argc, const char **argv)
{
    ap_t *ap = ap_new("cli", NEW_VERSION(0, 0, 1));

    runtime_t rt = {
        .argc = argc,
        .argv = argv,

        .mediator = mediator,
        .lifetime = lifetime,

        .reports = reports,
        .ap = ap,

        .warnAsError = false,
        .reportLimit = 20,

        .sourcePaths = vector_new(16),
    };

    langs_t langs = get_langs();
    for (size_t i = 0; i < langs.size; i++)
    {
        lifetime_config_language(lifetime, ap, langs.langs + i);
    }

    for (size_t i = 0; i < langs.size; i++)
    {
        lifetime_add_language(lifetime, langs.langs + i);
    }

    ap_group_t *generalGroup = ap_group_new(ap, "general", "general options");
    ap_param_t *helpParam = ap_add_bool(generalGroup, "help", "display this message", kHelpNames);
    ap_param_t *versionParam = ap_add_bool(generalGroup, "version", "print version information", kVersionNames);
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
    ap_event(ap, helpParam, on_help, &rt);
    ap_event(ap, versionParam, on_version, &rt);
    ap_event(ap, addExtensionMapParam, on_register_ext, &rt);

    // debug
    ap_event(ap, debugSsaParam, on_set_debug_ssa, &rt);
    ap_event(ap, debugVerboseParam, on_set_verbose, &rt);

    ap_event(ap, outputFileParam, on_set_source, &rt);
    ap_event(ap, outputHeaderParam, on_set_header, &rt);

    ap_event(ap, NULL, on_add_source, &rt);
    ap_error(ap, on_arg_error, &rt);

    ap_parse(ap, reports, argc, argv);

    return rt;
}
