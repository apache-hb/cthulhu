#include "base/version-def.h"
#include "platform/library.h"
#include "platform/error.h"

#include "report/report.h"
#include "std/vector.h"
#include "std/str.h"

#include "argparse/argparse.h"
#include "argparse/commands.h"

#include "cthulhu/mediator/mediator.h"

#include "version.h"

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

typedef struct format_entry_t {
    const char *name;
    out_format_t fmt;
} format_entry_t;

static const char *kLoadPluginNames[] = { "-plugin", "--load-plugin", NULL };
static const char *kLoadLangNames[] = { "-lang", "--load-lang", NULL };

static const char *kFormatNames[] = { "-f", "--format", NULL };

static const format_entry_t kFormatEntries[] = {
    { "text", eFormatText },
    { "json", eFormatJson },
    { "xml", eFormatXml },
    { "toml", eFormatToml },
};

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

    for (size_t i = 0; i < eFormatTotal; i++)
    {
        format_entry_t entry = kFormatEntries[i];
        if (str_equal(entry.name, fmt))
        {
            logverbose("setting output format to `%s`", fmt);
            info->fmt = entry.fmt;
            return eEventHandled;
        }
    }
    
    fprintf(stderr, "Unknown format '%s'\n", fmt);

    return eEventHandled;
}

static AP_ERROR(on_error, ap, node, message, data)
{
    fprintf(stderr, "Error: %s\n", message);
    return eEventHandled;
}

static version_t get_version(version_info_t info)
{
    return info.version;
}

static void write_query_text(vector_t *langs, vector_t *plugins)
{
    size_t numPlugins = vector_len(plugins);
    size_t numLanguages = vector_len(langs);

    printf("query(plugins=%zu, languages=%zu)\n", numPlugins, numLanguages);

    for (size_t i = 0; i < numPlugins; i++)
    {
        if (i != 0)
        {
            printf("\n");
        }
        const plugin_t *plugin = vector_get(plugins, i);
        version_t version = get_version(plugin->version);

        printf("plugin(id=%s,name=%s,version=%" PRI_VERSION ".%" PRI_VERSION ".%" PRI_VERSION ")\n", plugin->id, plugin->name, VERSION_MAJOR(version), VERSION_MINOR(version), VERSION_PATCH(version));

        printf(" - configure: %s\n", plugin->fnConfigure ? "provided" : "default");
        printf(" - init: %s\n", plugin->fnInit ? "provided" : "default");
        printf(" - shutdown: %s\n", plugin->fnShutdown ? "provided" : "default");
        printf(" - region: %s\n", plugin->fnRegion ? "provided" : "default");
    }

    for (size_t i = 0; i < numLanguages; i++)
    {
        if (i != 0)
        {
            printf("\n");
        }
        const language_t *lang = vector_get(langs, i);
        version_t version = get_version(lang->version);

        printf("language(id=%s,name=%s,version=%" PRI_VERSION ".%" PRI_VERSION ".%" PRI_VERSION ")\n", lang->id, lang->name, VERSION_MAJOR(version), VERSION_MINOR(version), VERSION_PATCH(version));

        printf(" - configure: %s\n", lang->fnConfigure ? "provided" : "default");
        printf(" - init: %s\n", lang->fnInit ? "provided" : "default");
        printf(" - shutdown: %s\n", lang->fnShutdown ? "provided" : "default");

        printf(" - parse: %s\n", lang->fnParse ? "provided" : "default");
        printf(" - forward: %s\n", lang->fnForward ? "provided" : "default");
        printf(" - import: %s\n", lang->fnImport ? "provided" : "default");
        printf(" - compile: %s\n", lang->fnCompile ? "provided" : "default");
    }
}

static void write_query_toml(version_t version, vector_t *langs, vector_t *plugins)
{
    size_t numLangs = vector_len(langs);
    size_t numPlugins = vector_len(plugins);

    printf("[info]\n");
    printf("interface-version = { major = %" PRI_VERSION ", minor = %" PRI_VERSION ", patch = %" PRI_VERSION " }\n", VERSION_MAJOR(version), VERSION_MINOR(version), VERSION_PATCH(version));
    printf("framework-version = { major = %" PRI_VERSION ", minor = %" PRI_VERSION ", patch = %" PRI_VERSION " }\n", CTHULHU_MAJOR, CTHULHU_MINOR, CTHULHU_PATCH);

    if (numLangs > 0)
    {
        printf("\n[languages]\n");
    }

    for (size_t i = 0; i < numLangs; ++i)
    {
        printf("\n");

        const language_t *lang = vector_get(langs, i);
        version_t version = get_version(lang->version);

        printf("[languages.%s]\n", lang->id);
        printf("name = \"%s\"\n", lang->name);
        printf("version = { major = %" PRI_VERSION ", minor = %" PRI_VERSION ", patch = %" PRI_VERSION " }\n", VERSION_MAJOR(version), VERSION_MINOR(version), VERSION_PATCH(version));
        printf("configure = \"%s\"\n", lang->fnConfigure ? "true" : "false");
        printf("init = %s\n", lang->fnInit ? "true" : "false");
        printf("shutdown = %s\n", lang->fnShutdown ? "true" : "false");
        printf("parse = %s\n", lang->fnParse ? "true" : "false");
        printf("forward = %s\n", lang->fnForward ? "true" : "false");
        printf("import = %s\n", lang->fnImport ? "true" : "false");
        printf("compile = %s\n", lang->fnCompile ? "true" : "false");
    }

    if (numLangs > 0 && numPlugins > 0)
    {
        printf("\n");
    }

    if (numPlugins > 0)
    {
        printf("[plugins]\n");
    }

    for (size_t i = 0; i < numPlugins; ++i)
    {
        printf("\n");

        const plugin_t *plugin = vector_get(plugins, i);
        version_t version = get_version(plugin->version);

        printf("[plugins.%s]\n", plugin->id);
        printf("name = \"%s\"\n", plugin->name);
        printf("version = { major = %" PRI_VERSION ", minor = %" PRI_VERSION ", patch = %" PRI_VERSION " }\n", VERSION_MAJOR(version), VERSION_MINOR(version), VERSION_PATCH(version));
        printf("configure = \"%s\"\n", plugin->fnConfigure ? "true" : "false");
        printf("init = %s\n", plugin->fnInit ? "true" : "false");
        printf("shutdown = %s\n", plugin->fnShutdown ? "true" : "false");
        printf("region = %s\n", plugin->fnRegion ? "true" : "false");
    }
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
    version_t version = NEW_VERSION(1, 0, 0);
    ap_t *ap = ap_new("Cthulhu query interface", version);

    ap_group_t *general = ap_group_new(ap, "General", "General options");
    ap_param_t *loadPluginParam = ap_add_string(general, "add plugin", "load a plugin from a shared library to query its info", kLoadPluginNames);
    ap_param_t *loadLanguageParam = ap_add_string(general, "add language", "load a language from a shared library to query its info", kLoadLangNames);

    ap_group_t *query = ap_group_new(ap, "Query", "Query options");
    ap_param_t *formatParam = ap_add_string(query, "output format", "output format of the query [text|json|xml|toml] (default: text)", kFormatNames);

    ap_event(ap, loadPluginParam, on_load_plugin, &info);
    ap_event(ap, loadLanguageParam, on_lang_plugin, &info);
    ap_event(ap, formatParam, on_set_format, &info);

    ap_error(ap, on_error, &info);

    ap_parse(ap, reports, argc, argv);

    switch (info.fmt)
    {
    case eFormatToml: 
        write_query_toml(version, info.languages, info.plugins); 
        break;
        
    case eFormatText:
    default: 
        write_query_text(info.languages, info.plugins); 
        break;
    }
}
