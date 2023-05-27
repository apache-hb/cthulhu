#include "cmd.h"

#include "cthulhu/mediator/mediator.h"
#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"

#include "argparse/argparse.h"
#include "argparse/commands.h"

#include "io/io.h"

#include "report/report.h"

#include "cthulhu/ssa/ssa.h"

#include "cthulhu/emit/c89.h"

#include <stdio.h>

#define CHECK_REPORTS(reports, msg) \
    do { \
        int err = end_reports(reports, msg, reportConfig); \
        if (err != 0) { \
            return err; \
        } \
    } while (0)

static void add_sources(mediator_t *mediator, lifetime_t *lifetime, vector_t *sources)
{
    size_t len = vector_len(sources);
    for (size_t i = 0; i < len; i++)
    {
        const char *path = vector_get(sources, i);
        const char *ext = str_ext(path);
        if (ext == NULL)
        {
            printf("could not identify compiler for `%s` (no extension)\n", path);
            continue;
        }

        const language_t *lang = mediator_get_language_by_ext(mediator, ext);
        if (lang == NULL)
        {
            printf("could not identify compiler for `%s` by extension `%s`.\nnote: extra extensions can be provided with -ext=id:ext", path, ext);
            continue;
        }

        io_t *io = io_file(path, eFileRead | eFileText);
        if (io_error(io) != 0)
        {
            printf("failed to load source `%s`\n%s\n", path, error_string(io_error(io)));
            continue;
        }

        source_t src = {
            .io = io,
            .lang = lang
        };

        lifetime_add_source(lifetime, src);
    }
}

static io_t *make_file(const char *path)
{
    io_t *io = io_file(path, eFileWrite | eFileText);
    if (io_error(io) != 0)
    {
        printf("failed to open `%s` for writing\n%s\n", path, error_string(io_error(io)));
        return NULL;
    }

    return io;
}

int main(int argc, const char **argv)
{
    mediator_t *mediator = mediator_new("cli", NEW_VERSION(0, 0, 1));
    lifetime_t *lifetime = mediator_get_lifetime(mediator);

    runtime_t rt = cmd_parse(mediator, argc, argv);

    size_t errs = vector_len(rt.unknownArgs);
    for (size_t i = 0; i < errs; i++)
    {
        const char *err = vector_get(rt.unknownArgs, i);
        printf("error: %s\n", err);
    }

    size_t len = vector_len(rt.sourcePaths);
    if (len == 0)
    {
        printf("no source files provided\n");
    }
    else
    {
        add_sources(mediator, lifetime, rt.sourcePaths);
    }

    report_config_t reportConfig = {
        .limit = rt.reportLimit,
        .warningsAreErrors = rt.warnAsError
    };

    lifetime_init(lifetime);

    lifetime_parse(rt.reports, lifetime);
    CHECK_REPORTS(rt.reports, "failed to parse sources");

    lifetime_forward(rt.reports, lifetime);
    CHECK_REPORTS(rt.reports, "failed to forward declarations");

    lifetime_compile(rt.reports, lifetime);
    CHECK_REPORTS(rt.reports, "failed to compile sources");

    vector_t *mods = lifetime_modules(lifetime);
    ssa_module_t *ssa = ssa_gen_module(rt.reports, mods);
    CHECK_REPORTS(rt.reports, "failed to generate SSA");

    ssa_opt_module(rt.reports, ssa);
    CHECK_REPORTS(rt.reports, "failed to optimize SSA");

    if (rt.emitSSA)
    {
        ssa_emit_module(rt.reports, ssa);
    }

    const char *sourcePath = rt.sourceOut == NULL ? "out.c" : format("%s.c", rt.sourceOut);
    const char *headerPath = rt.headerOut == NULL ? "out.h" : format("%s.h", rt.headerOut);

    io_t *src = make_file(sourcePath);
    io_t *header = rt.headerOut == NULL ? NULL : make_file(headerPath);

    emit_config_t config = {
        .reports = rt.reports,
        .source = src,
        .header = header
    };

    c89_emit_ssa_modules(config, ssa);
    CHECK_REPORTS(rt.reports, "failed to emit C89");

    lifetime_deinit(lifetime);

    size_t langs = vector_len(rt.languages);
    size_t plugins = vector_len(rt.plugins);

    for (size_t i = 0; i < langs; i++)
    {
        const language_t *lang = vector_get(rt.languages, i);
        mediator_unload_language(rt.mediator, lang);
    }

    for (size_t i = 0; i < plugins; i++)
    {
        const plugin_t *plugin = vector_get(rt.plugins, i);
        mediator_unload_plugin(rt.mediator, plugin);
    }
}
