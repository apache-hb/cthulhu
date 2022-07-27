#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/interface/interface.h"
#include "cthulhu/interface/runtime.h"
#include "report/report.h"
#include "scan/compile.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"

#include "io/io.h"

#include "std/str.h"

static status_t report_errors(cthulhu_t *cthulhu, const char *name)
{
    status_t status = end_reports(cthulhu->reports, name, cthulhu->config.reportConfig);
    cthulhu->status = status;
    return status;
}

static bool end_stage(cthulhu_t *cthulhu, const char *name)
{
    return report_errors(cthulhu, name) != EXIT_OK;
}

static void rename_module(hlir_t *hlir, const char *path)
{
    if (hlir->name != NULL)
    {
        return;
    }

    hlir->name = str_filename(path);
}

static runtime_t runtime_new(reports_t *reports, size_t size)
{
    runtime_t runtime = {
        .reports = reports,
        .modules = map_optimal(size),
    };

    return runtime;
}

static size_t total_sources(const cthulhu_t *cthulhu)
{
    CTASSERT(cthulhu != NULL);

    return vector_len(cthulhu->sources);
}

static compile_t *get_compile(const cthulhu_t *cthulhu, size_t idx)
{
    CTASSERT(idx <= total_sources(cthulhu));

    return cthulhu->compiles + idx;
}

cthulhu_t *cthulhu_new(driver_t driver, vector_t *sources, config_t config)
{

    CTASSERT(driver.fnInitCompiler != NULL);
    CTASSERT(driver.fnParseFile != NULL);
    CTASSERT(driver.fnForwardDecls != NULL);
    CTASSERT(driver.fnResolveImports != NULL);
    CTASSERT(driver.fnCompileModule != NULL);

    cthulhu_t *cthulhu = ctu_malloc(sizeof(cthulhu_t));

    reports_t *reports = begin_reports();

    size_t totalSources = vector_len(sources);
    runtime_t runtime = runtime_new(reports, totalSources);

    cthulhu->driver = driver;
    cthulhu->config = config;
    cthulhu->status = EXIT_INTERNAL;
    cthulhu->reports = reports;
    cthulhu->sources = sources;
    cthulhu->runtime = runtime;

    return cthulhu;
}

status_t cthulhu_init(cthulhu_t *cthulhu)
{
    cthulhu->driver.fnInitCompiler(&cthulhu->runtime);

    size_t totalSources = total_sources(cthulhu);

    cthulhu->compiles = ctu_malloc(totalSources * sizeof(compile_t));

    for (size_t i = 0; i < totalSources; i++)
    {
        io_t *source = vector_get(cthulhu->sources, i);

        if (io_error(source) != 0)
        {
            message_t *id =
                report(cthulhu->reports, eFatal, node_invalid(), "failed to open file `%s`", io_name(source));
            report_note(id, "%s", error_string(io_error(source)));
            continue;
        }

        scan_config_t config = {
            .alloc = &globalAlloc,
            .nodeAlloc = &globalAlloc,
            .astAlloc = &globalAlloc,
            .yyAlloc = &globalAlloc
        };

        compile_t *ctx = get_compile(cthulhu, i);
        ctx->ast = NULL;
        ctx->hlir = NULL;
        ctx->scan = scan_io(cthulhu->reports, cthulhu->driver.name, source, config);
    }

    return report_errors(cthulhu, "scanning sources");
}

status_t cthulhu_parse(cthulhu_t *cthulhu)
{
    size_t totalSources = total_sources(cthulhu);

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = get_compile(cthulhu, i);
        ctx->ast = cthulhu->driver.fnParseFile(&cthulhu->runtime, ctx);
    }

    return report_errors(cthulhu, "parsing sources");
}

status_t cthulhu_forward(cthulhu_t *cthulhu)
{
    size_t totalSources = total_sources(cthulhu);

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = get_compile(cthulhu, i);
        cthulhu->driver.fnForwardDecls(&cthulhu->runtime, ctx);
    }

    if (end_stage(cthulhu, "forwarding declarations"))
    {
        return cthulhu->status;
    }

    for (size_t i = 0; i < totalSources; i++)
    {
        io_t *source = vector_get(cthulhu->sources, i);
        compile_t *ctx = get_compile(cthulhu, i);
        hlir_t *hlir = ctx->hlir;

        rename_module(hlir, io_name(source));
        add_module(&cthulhu->runtime, get_hlir_name(hlir), ctx->sema);
    }

    return report_errors(cthulhu, "forwarding declarations");
}

status_t cthulhu_resolve(cthulhu_t *cthulhu)
{
    size_t totalSources = total_sources(cthulhu);

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = get_compile(cthulhu, i);
        cthulhu->driver.fnResolveImports(&cthulhu->runtime, ctx);
    }

    return report_errors(cthulhu, "import resolution");
}

status_t cthulhu_compile(cthulhu_t *cthulhu)
{
    size_t totalSources = total_sources(cthulhu);

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = get_compile(cthulhu, i);
        cthulhu->driver.fnCompileModule(&cthulhu->runtime, ctx);
    }

    if (end_stage(cthulhu, "compiling modules"))
    {
        return cthulhu->status;
    }

    check_t check = {
        .reports = cthulhu->reports,
    };

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = get_compile(cthulhu, i);
        check_module(&check, ctx->hlir);
    }

    return report_errors(cthulhu, "validation");
}

vector_t *cthulhu_get_modules(cthulhu_t *cthulhu)
{
    size_t totalSources = total_sources(cthulhu);

    vector_t *result = vector_of(totalSources);

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = get_compile(cthulhu, i);
        vector_set(result, i, ctx->hlir);
    }

    return result;
}
