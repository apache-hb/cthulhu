#include "platform/library.h"
#include "platform/error.h"

#include "report/report.h"
#include "std/vector.h"
#include "std/str.h"

#include "argparse2/argparse.h"
#include "argparse2/commands.h"

#include "cthulhu/mediator/mediator.h"

#include <stdio.h>

typedef enum out_format_t
{
    eFormatText,
    eFormatJson,
    eFormatXml,
    eFormatToml,
    // eFormatHtml,

    eFormatTotal
} out_format_t;

typedef struct query_t
{
    int argc;
    const char **argv;

    out_format_t fmt;

    vector_t *plugins;
    vector_t *languages;
} query_t;

static const char *kLoadPluginNames[] = { "-plugin", "--load-plugin", NULL };
static const char *kLoadLangNames[] = { "-lang", "--load-lang", NULL };

static const char *kFormatNames[] = { "-f", "--format", NULL };

static AP_EVENT(on_load_plugin, ap, param, value, data)
{
    query_t *info = data;
    const char *path = value;

    cerror_t err = 0;
    library_t *lib = library_open(path, &err);
    if (err != 0)
    {
        fprintf(stderr, "Failed to load plugin '%s': %s\n", path, error_string(err));
        return eEventHandled;
    }

    plugin_load_t load = library_get(lib, STR(PLUGIN_ENTRY_POINT), &err);
    if (err != 0)
    {
        fprintf(stderr, "Failed to load plugin entrypoint '%s': %s\n", path, error_string(err));
        return eEventHandled;
    }

    // TODO: passing null is a little dangerous
    const plugin_t *plugin = load(NULL);

    vector_push(&info->plugins, (plugin_t*)plugin);

    return eEventHandled;
}

static AP_EVENT(on_lang_plugin, ap, param, value, data)
{
    query_t *info = data;
    const char *path = value;

    cerror_t err = 0;
    library_t *lib = library_open(path, &err);
    if (err != 0)
    {
        fprintf(stderr, "Failed to load language '%s': %s\n", path, error_string(err));
        return eEventHandled;
    }

    language_load_t load = library_get(lib, STR(LANGUAGE_ENTRY_POINT), &err);
    if (err != 0)
    {
        fprintf(stderr, "Failed to load language entrypoint '%s': %s\n", path, error_string(err));
        return eEventHandled;
    }

    const language_t *lang = load(NULL);

    vector_push(&info->languages, (language_t*)lang);

    return eEventHandled;
}

static AP_EVENT(on_set_format, ap, param, value, data)
{
    query_t *info = data;
    const char *fmt = value;

    // TODO: string lookup tables would be nice
    if (str_equal(fmt, "text") == 0)
    {
        info->fmt = eFormatText;
    }
    else if (str_equal(fmt, "json") == 0)
    {
        info->fmt = eFormatJson;
    }
    else if (str_equal(fmt, "xml") == 0)
    {
        info->fmt = eFormatXml;
    }
    else if (str_equal(fmt, "toml") == 0)
    {
        info->fmt = eFormatToml;
    }
    else
    {
        fprintf(stderr, "Unknown format '%s'\n", fmt);
    }

    return eEventHandled;
}

static AP_ERROR(on_error, ap, node, message, data)
{
    fprintf(stderr, "Error: %s\n", message);
    return eEventHandled;
}

int main(int argc, const char **argv)
{
    query_t info = {
        .argc = argc,
        .argv = argv,

        .fmt = eFormatText,

        .plugins = vector_new(16),
        .languages = vector_new(16),
    };

    reports_t *reports = begin_reports();
    ap_t *ap = ap_new("Cthulhu query interface", NEW_VERSION(1, 0, 0));

    ap_group_t *general = ap_group_new(ap, "General", "General options");
    ap_param_t *loadPluginParam = ap_add_string(general, "load a plugin", kLoadPluginNames);
    ap_param_t *loadLanguageParam = ap_add_string(general, "load a language", kLoadLangNames);

    ap_group_t *query = ap_group_new(ap, "Query", "Query options");
    ap_param_t *formatParam = ap_add_string(query, "output format of the query [text|json|xml|toml] (default: text)", kFormatNames);

    ap_event(ap, loadPluginParam, on_load_plugin, &info);
    ap_event(ap, loadLanguageParam, on_lang_plugin, &info);
    ap_event(ap, formatParam, on_set_format, &info);

    ap_error(ap, on_error, &info);

    ap_parse(ap, reports, argc, argv);

    size_t numPlugins = vector_len(info.plugins);
    size_t numLanguages = vector_len(info.languages);

    printf("query(plugins=%zu, languages=%zu)\n", numPlugins, numLanguages);

    for (size_t i = 0; i < numPlugins; i++)
    {
        const plugin_t *plugin = vector_get(info.plugins, i);
        
        printf("plugin(id=%s,name=%s,version=%" PRI_VERSION ".%" PRI_VERSION ".%" PRI_VERSION ")\n", plugin->id, plugin->name, VERSION_MAJOR(plugin->version), VERSION_MINOR(plugin->version), VERSION_PATCH(plugin->version));
        
        printf(" - configure: %s\n", plugin->fnConfigure ? "provided" : "default");
        printf(" - init: %s\n", plugin->fnInit ? "provided" : "default");
        printf(" - shutdown: %s\n", plugin->fnShutdown ? "provided" : "default");
        printf(" - region: %s\n", plugin->fnRegion ? "provided" : "default");
    }

    for (size_t i = 0; i < numLanguages; i++)
    {
        const language_t *lang = vector_get(info.languages, i);

        printf("language(id=%s,name=%s,version=%" PRI_VERSION ".%" PRI_VERSION ".%" PRI_VERSION ")\n", lang->id, lang->name, VERSION_MAJOR(lang->version), VERSION_MINOR(lang->version), VERSION_PATCH(lang->version));

        printf(" - configure: %s\n", lang->fnConfigure ? "provided" : "default");
        printf(" - init: %s\n", lang->fnInit ? "provided" : "default");
        printf(" - shutdown: %s\n", lang->fnShutdown ? "provided" : "default");

        printf(" - parse: %s\n", lang->fnParse ? "provided" : "default");
        printf(" - forward: %s\n", lang->fnForward ? "provided" : "default");
        printf(" - import: %s\n", lang->fnImport ? "provided" : "default");
        printf(" - compile: %s\n", lang->fnCompile ? "provided" : "default");
    }
}
