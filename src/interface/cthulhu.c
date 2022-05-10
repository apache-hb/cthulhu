#include "cthulhu/interface/interface.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/interface/runtime.h"
#include "cthulhu/util/report.h"
#include "cthulhu/ast/compile.h"

static int report_errors(cthulhu_t *cthulhu, const char *name)
{
    int status = end_reports(cthulhu->reports, name, &cthulhu->config.reportConfig);
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
        .modules = map_optimal(size)
    };

    return runtime;
}

cthulhu_t *cthulhu_new(driver_t driver, vector_t *sources, config_t config)
{
    size_t totalSources = vector_len(sources);
    reports_t *reports = begin_reports();
    runtime_t runtime = runtime_new(reports, totalSources);

    cthulhu_t *cthulhu = ctu_malloc(sizeof(cthulhu_t));

    cthulhu->driver = driver;
    cthulhu->config = config;
    cthulhu->status = EXIT_INTERNAL;
    cthulhu->reports = reports;

    cthulhu->runtime = runtime;
    cthulhu->compiles = vector_new(totalSources);

    cthulhu->sources = sources;

    return cthulhu;
}

int cthulhu_init(cthulhu_t *cthulhu)
{
    cthulhu->driver.fnInitCompiler(&cthulhu->runtime);

    size_t totalSources = vector_len(cthulhu->sources);

    cthulhu->compiles = vector_of(totalSources);

    for (size_t i = 0; i < totalSources; i++)
    {
        const char *path = vector_get(cthulhu->sources, i);

        compile_t *ctx = ctu_malloc(sizeof(compile_t));
        ctx->ast = NULL;
        ctx->hlir = NULL;

        error_t error = 0;
        file_t handle = file_open(path, 0, &error);

        if (error != 0)
        {
            message_t *id = report(cthulhu->reports, ERROR, NULL, "failed to open file `%s`", path);
            report_note(id, "%s", error_string(error));
            continue;
        }

        ctx->file = handle;
        vector_set(cthulhu->compiles, i, ctx);
    }

    if (end_stage(cthulhu, "opening sources"))
    {
        return cthulhu->status;
    }
    
    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = vector_get(cthulhu->compiles, i);
        scan_t scanner = scan_file(cthulhu->reports, cthulhu->driver.name, ctx->file);
        ctx->scanner = BOX(scanner);
    }

    return report_errors(cthulhu, "scanning sources");
}

int cthulhu_parse(cthulhu_t *cthulhu)
{
    size_t totalSources = vector_len(cthulhu->sources);

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = vector_get(cthulhu->compiles, i);
        cthulhu->driver.fnParseFile(&cthulhu->runtime, ctx);
    }

    return report_errors(cthulhu, "parsing sources");
}

int cthulhu_forward(cthulhu_t *cthulhu)
{
    size_t totalSources = vector_len(cthulhu->sources);

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = vector_get(cthulhu->compiles, i);
        cthulhu->driver.fnForwardDecls(&cthulhu->runtime, ctx);
    }

    if (end_stage(cthulhu, "forwarding declarations"))
    {
        return cthulhu->status;
    }

    for (size_t i = 0; i < totalSources; i++)
    {
        const char *path = vector_get(cthulhu->sources, i);
        compile_t *ctx = vector_get(cthulhu->compiles, i);
        hlir_t *hlir = ctx->hlir;

        rename_module(hlir, path);
        add_module(&cthulhu->runtime, hlir);
    }

    return report_errors(cthulhu, "forwarding declarations");
}

int cthulhu_resolve(cthulhu_t *cthulhu)
{
    size_t totalSources = vector_len(cthulhu->sources);

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = vector_get(cthulhu->compiles, i);
        cthulhu->driver.fnResolveImports(&cthulhu->runtime, ctx);
    }

    return report_errors(cthulhu, "import resolution");
}

int cthulhu_compile(cthulhu_t *cthulhu)
{
    size_t totalSources = vector_len(cthulhu->sources);

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = vector_get(cthulhu->compiles, i);
        cthulhu->driver.fnCompileModule(&cthulhu->runtime, ctx);
    }

    if (end_stage(cthulhu, "compiling modules"))
    {
        return cthulhu->status;
    }

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = vector_get(cthulhu->compiles, i);
        check_module(cthulhu->reports, ctx->hlir);
    }

    return report_errors(cthulhu, "validation");
}

vector_t *cthulhu_get_modules(cthulhu_t *cthulhu)
{
    size_t totalSources = vector_len(cthulhu->sources);

    vector_t *result = vector_of(totalSources);

    for (size_t i = 0; i < totalSources; i++)
    {
        compile_t *ctx = vector_get(cthulhu->compiles, i);
        vector_set(result, i, ctx->hlir);
    }

    return result;
}
