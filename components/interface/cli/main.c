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

#include "support/langs.h"

#define CHECK_REPORTS(reports, msg) \
    do { \
        int err = end_reports(reports, msg, reportConfig); \
        if (err != 0) { \
            return err; \
        } \
    } while (0)

static void add_sources(mediator_t *mediator, lifetime_t *lifetime, vector_t *sources, reports_t *reports)
{
    size_t len = vector_len(sources);
    for (size_t i = 0; i < len; i++)
    {
        const char *path = vector_get(sources, i);
        const char *ext = str_ext(path);
        if (ext == NULL)
        {
            report(reports, eFatal, NULL, "could not identify compiler for `%s` (no extension)", path);
            continue;
        }

        const language_t *lang = mediator_get_language_by_ext(mediator, ext);
        if (lang == NULL)
        {
            const char *basepath = str_filename(path);
            message_t *id = report(reports, eFatal, NULL, "could not identify compiler for `%s` by extension `%s`", basepath, ext);
            report_note(id, "extra extensions can be provided with -ext=id:ext");
            continue;
        }

        io_t *io = io_file(path, eFileRead | eFileText);
        if (io_error(io) != 0)
        {
            report(reports, eFatal, NULL, "failed to load source `%s`\n%s", path, error_string(io_error(io)));
            continue;
        }

        source_t src = {
            .io = io,
            .lang = lang
        };

        lifetime_add_source(lifetime, src);
    }
}

static io_t *make_file(reports_t *reports, const char *path)
{
    io_t *io = io_file(path, eFileWrite | eFileText);
    if (io_error(io) != 0)
    {
        report(reports, eFatal, NULL, "failed to open `%s` for writing\n%s", path, error_string(io_error(io)));
        return NULL;
    }

    return io;
}

int main(int argc, const char **argv)
{
    reports_t *reports = begin_reports();
    mediator_t *mediator = mediator_new("cli", NEW_VERSION(0, 0, 1));
    lifetime_t *lifetime = mediator_get_lifetime(mediator, reports);

    langs_t langs = get_langs();
    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        mediator_load_language(mediator, lang);
    }

    runtime_t rt = cmd_parse(reports, mediator, argc, argv);
    report_config_t reportConfig = {
        .limit = rt.reportLimit,
        .warningsAreErrors = rt.warnAsError
    };

    CHECK_REPORTS(reports, "failed to parse command line arguments");

    size_t len = vector_len(rt.sourcePaths);
    if (len == 0)
    {
        report(reports, eFatal, NULL, "no source files provided");
    }
    else
    {
        add_sources(mediator, lifetime, rt.sourcePaths, reports);
    }

    CHECK_REPORTS(reports, "failed to load sources");

    lifetime_init(lifetime);

    lifetime_parse(reports, lifetime);
    CHECK_REPORTS(reports, "failed to parse sources");

    lifetime_forward(reports, lifetime);
    CHECK_REPORTS(reports, "failed to forward declarations");

    lifetime_import(reports, lifetime);
    CHECK_REPORTS(reports, "failed to import modules");

    lifetime_compile(reports, lifetime);
    CHECK_REPORTS(reports, "failed to compile sources");

    vector_t *mods = lifetime_modules(lifetime);
    ssa_module_t *ssa = ssa_gen_module(reports, mods);
    CHECK_REPORTS(reports, "failed to generate SSA");

    ssa_opt_module(reports, ssa);
    CHECK_REPORTS(reports, "failed to optimize SSA");

    if (rt.emitSSA)
    {
        ssa_emit_module(reports, ssa);
    }

    const char *sourcePath = rt.sourceOut == NULL ? "out.c" : format("%s.c", rt.sourceOut);

    io_t *src = make_file(reports, sourcePath);
    io_t *header = rt.headerOut == NULL ? NULL : make_file(reports, format("%s.h", rt.headerOut));

    CHECK_REPORTS(reports, "failed to open output files");

    emit_config_t config = {
        .reports = reports,
        .source = src,
        .header = header
    };

    c89_emit_ssa_modules(config, ssa);
    CHECK_REPORTS(reports, "failed to emit C89");

    lifetime_deinit(lifetime);

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        mediator_unload_language(rt.mediator, lang);
    }
}
