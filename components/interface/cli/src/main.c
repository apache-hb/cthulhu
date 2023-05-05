#include "base/util.h"

#include "std/vector.h"
#include "std/str.h"

#include "platform/library.h"
#include "platform/error.h"

#include "report/report.h"

#include "argparse/argparse.h"
#include "argparse/commands.h"

#include "cthulhu/mediator/mediator.h"

#include "io/io.h"

#include <stdio.h>

typedef struct runtime_t 
{
    int argc;
    const char **argv;

    reports_t *reports;
    ap_t *ap;
    mediator_t *mediator;

    vector_t *languages;
    vector_t *plugins;

    vector_t *sourcePaths;

    vector_t *unknownArgs;
} runtime_t;

typedef struct compile_t
{
    io_t *io;
    const language_t *lang;
} compile_t;

// general
static const char *kHelpNames[] = { "-h", "--help", NULL };
static const char *kVersionNames[] = { "-v", "--version", NULL };
static const char *kLoadLangNames[] = { "-lang", "--load-lang", NULL };
static const char *kLoadPluginNames[] = { "-plugin", "--load-plugin", NULL };
static const char *kAddExtensionMapNames[] = { "-ext", "--add-ext", NULL };

// codegen
static const char *kOutputFileNames[] = { "-o", "--output", NULL };
static const char *kOutputGenNames[] = { "-cg", "--codegen", NULL };
static const char *kOutputHeaderNames[] = { "-header", "--output-header", NULL };

// debug
static const char *kDebugSsaNames[] = { "-dbgssa", "--debug-ssa", NULL };
static const char *kDebugVerboseNames[] = { "-V", "--verbose", NULL };

static AP_EVENT(on_help, ap, param, value, data)
{
    runtime_t *rt = data;
    ap_print_help_header(ap, rt->argv[0]);

    size_t langCount = vector_len(rt->languages);
    if (langCount != 0)
    {
        printf("\n%zu languages loaded:\n", langCount);
        printf("========================================\n");
    }
    for (size_t i = 0; i < langCount; i++)
    {
        const language_t *lang = vector_get(rt->languages, i);
        ap_print_version_info(lang->version, lang->name);
        printf("========================================\n");
    }

    size_t pluginCount = vector_len(rt->plugins);
    if (pluginCount != 0)
    {
        printf("\n%zu plugins:\n", pluginCount);
        printf("========================================\n");
    }
    for (size_t i = 0; i < pluginCount; i++)
    {
        const plugin_t *plugin = vector_get(rt->plugins, i);
        ap_print_version_info(plugin->version, plugin->name);
        printf("========================================\n");
    }

    if (langCount != 0 || pluginCount != 0)
    {
        printf("\n");
    }

    ap_print_help_body(ap, rt->argv[0]);

    return eEventHandled;
}

static AP_EVENT(on_version, ap, param, value, data)
{
    ap_version(ap);

    return eEventHandled;
}

static AP_EVENT(on_load_language, ap, param, value, data)
{
    UNUSED(param);
    UNUSED(data);

    const char *path = value;
    runtime_t *rt = data;

    cerror_t err = 0;
    library_t *handle = library_open(path, &err);
    if (err != 0)
    {
        printf("failed to load language `%s`\n%s\n", path, error_string(err));
        return eEventHandled;
    }

    language_load_t load = library_get(handle, STR(LANGUAGE_ENTRY_POINT), &err);
    if (err != 0)
    {
        printf("failed to load language entrypoint `%s`\n%s\n", path, error_string(err));
        return eEventHandled;
    }

    const language_t *lang = load(rt->mediator);
    if (lang == NULL)
    {
        printf("failed to load language `%s` (language failed to load)\n", path);
        return eEventHandled;
    }

    vector_push(&rt->languages, (language_t*)lang);

    mediator_add_language(rt->mediator, lang, ap);

    return eEventHandled;
}

static AP_EVENT(on_load_plugin, ap, param, value, data)
{
    UNUSED(param);
    UNUSED(data);

    const char *path = value;
    runtime_t *rt = data;

    cerror_t err = 0;
    library_t *handle = library_open(path, &err);
    if (err != 0)
    {
        printf("failed to load plugin `%s`\n%s\n", path, error_string(err));
        return eEventHandled;
    }

    plugin_load_t load = library_get(handle, STR(PLUGIN_ENTRY_POINT), &err);
    if (err != 0)
    {
        printf("failed to load plugin entrypoint `%s`\n%s\n", path, error_string(err));
        return eEventHandled;
    }

    const plugin_t *plugin = load(rt->mediator);
    if (plugin == NULL)
    {
        printf("failed to load plugin `%s` (plugin failed to load)\n", path);
        return eEventHandled;
    }

    vector_push(&rt->plugins, (plugin_t*)plugin);

    mediator_add_plugin(rt->mediator, plugin, ap);

    return eEventHandled;
}

static AP_EVENT(on_register_ext, ap, param, value, data)
{
    UNUSED(ap);
    UNUSED(param);

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

    const language_t *lang = mediator_get_language(rt->mediator, id);
    if (lang == NULL)
    {
        printf("failed to register extension `%s` (no language identified by id=%s found)\n", mapping, id);
        return eEventHandled;
    }

    const language_t *old = mediator_register_extension(rt->mediator, ext, lang);
    if (old != NULL)
    {
        printf("failed to register extension `%s` (extension already registered by %s)\n", mapping, old->name);
        return eEventHandled;
    }

    logverbose("registered extension `%s` to language `%s`", ext, lang->name);

    return eEventHandled;
}

static AP_EVENT(on_add_source, ap, param, value, data)
{
    UNUSED(ap);
    UNUSED(param);
    UNUSED(data);

    const char *path = value;
    runtime_t *rt = data;

    vector_push(&rt->sourcePaths, (char*)path);

    return eEventHandled;
}

static AP_EVENT(on_set_verbose, ap, param, value, data)
{
    UNUSED(ap);
    UNUSED(param);
    UNUSED(data);

    const bool *val = value;

    verbose = *val;

    logverbose("%sd verbose logging", verbose ? "enable" : "disable");

    return eEventHandled;
}

static AP_ERROR(on_arg_error, ap, node, message, data)
{
    runtime_t *rt = data;

    vector_push(&rt->unknownArgs, (char*)message);

    return eEventHandled;
}

int main(int argc, const char **argv)
{
    runtime_init();
    reports_t *reports = begin_reports();
    mediator_t *mediator = mediator_new("cli", NEW_VERSION(0, 0, 1));
    ap_t *ap = ap_new("cli compiler", NEW_VERSION(0, 0, 1));

    mediator_region(mediator, eRegionLoadCompiler);

    runtime_t rt = {
        .argc = argc,
        .argv = argv,

        .reports = reports,
        .ap = ap,
        .mediator = mediator,
        
        .languages = vector_new(16),
        .plugins = vector_new(16),

        .sourcePaths = vector_new(16),
        .unknownArgs = vector_new(16),
    };

    ap_group_t *general = ap_group_new(ap, "general", "general options");
    ap_param_t *helpParam = ap_add_bool(general, "help", "display this message", kHelpNames);
    ap_param_t *versionParam = ap_add_bool(general, "version", "print version information", kVersionNames);
    ap_param_t *loadLanguageParam = ap_add_string(general, "load language", "load a new language driver", kLoadLangNames);
    ap_param_t *loadPluginParam = ap_add_string(general, "load plugin", "load a new shared library plugin", kLoadPluginNames);
    ap_param_t *addExtensionMapParam = ap_add_string(general, "add extension map", "register a new extension for a compiler", kAddExtensionMapNames);

    ap_group_t *codegen = ap_group_new(ap, "codegen", "code generation options");
    ap_param_t *outputFileParam = ap_add_string(codegen, "output file name", "output file name, will have .c appened to it (default: out)", kOutputFileNames);
    ap_param_t *outputGenParam = ap_add_string(codegen, "output generator", "code generator output to use [ssa-c89, hlir-c89] (default: ssa-c89)", kOutputGenNames);
    ap_param_t *outputHeaderParam = ap_add_string(codegen, "output header", "output header name, provide none to skip header generation. will have .c appeneded to it (default: none)", kOutputHeaderNames);

    ap_group_t *debug = ap_group_new(ap, "debug", "debug options");
    ap_param_t *debugSsaParam = ap_add_bool(debug, "debug ssa", "print debug ssa output", kDebugSsaNames);
    ap_param_t *debugVerboseParam = ap_add_bool(debug, "verbose", "enable verbose logging", kDebugVerboseNames);

    ap_event(ap, helpParam, on_help, &rt);
    ap_event(ap, versionParam, on_version, &rt);

    ap_event(ap, loadLanguageParam, on_load_language, &rt);
    ap_event(ap, loadPluginParam, on_load_plugin, &rt);
    ap_event(ap, addExtensionMapParam, on_register_ext, &rt);
    ap_event(ap, debugVerboseParam, on_set_verbose, &rt);
    ap_event(ap, NULL, on_add_source, &rt);
    ap_error(ap, on_arg_error, &rt);

    ap_parse(ap, reports, argc, argv);

    size_t errs = vector_len(rt.unknownArgs);
    for (size_t i = 0; i < errs; i++)
    {
        const char *err = vector_get(rt.unknownArgs, i);
        printf("error: %s\n", err);
    }

    mediator_region(mediator, eRegionInit);
    mediator_startup(mediator);

    mediator_region(mediator, eRegionLoadSource);

    size_t len = vector_len(rt.sourcePaths);
    vector_t *sources = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const char *path = vector_get(rt.sourcePaths, i);
        const char *ext = str_ext(path);
        if (ext == NULL)
        {
            printf("could not identify compiler for `%s` (no extension)\n", path);
            return 1;
        }

        const language_t *lang = mediator_get_language_for_ext(mediator, ext);
        if (lang == NULL)
        {
            printf("could not identify compiler for `%s` (no language registered for extension `%s`)\n", path, ext);
            return 1;
        }

        io_t *io = io_file(path, eFileRead | eFileText);
        if (io_error(io) != 0)
        {
            printf("failed to load source `%s`\n%s\n", path, error_string(io_error(io)));
            return 1;
        }
        
        vector_set(sources, i, io);
    }

    mediator_region(mediator, eRegionEnd);
    mediator_shutdown(mediator);
}
