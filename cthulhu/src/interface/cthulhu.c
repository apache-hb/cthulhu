#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/interface/interface.h"
#include "cthulhu/interface/runtime.h"
#include "report/report.h"
#include "scan/compile.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"

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

static alloc_t *get_alloc(const cthulhu_t *cthulhu)
{
    CTASSERT(cthulhu != NULL);

    return cthulhu->config.alloc;
}

static compile_t *get_compile(const cthulhu_t *cthulhu, size_t idx)
{
    CTASSERT(idx <= total_sources(cthulhu));

    return cthulhu->compiles + idx;
}

static source_t *source_of_kind(alloc_t *alloc, source_kind_t kind, const char *path)
{
    source_t *source = arena_malloc(alloc, sizeof(source_t), "source-code-bundle");
    source->kind = kind;
    source->path = path;
    return source;
}

source_t *source_file(alloc_t *alloc, const char *path)
{
    return source_of_kind(alloc, eSourceFile, path);
}

source_t *source_string(alloc_t *alloc, const char *path, const char *string)
{
    source_t *source = source_of_kind(alloc, eSourceString, path);
    source->string = string;
    return source;
}

cthulhu_t *cthulhu_new(driver_t driver, vector_t *sources, config_t config)
{
    CTASSERT(config.alloc != NULL);

    CTASSERT(driver.fnInitCompiler != NULL);
    CTASSERT(driver.fnParseFile != NULL);
    CTASSERT(driver.fnForwardDecls != NULL);
    CTASSERT(driver.fnResolveImports != NULL);
    CTASSERT(driver.fnCompileModule != NULL);

    cthulhu_t *cthulhu = arena_malloc(config.alloc, sizeof(cthulhu_t), "cthulhu-context");

    reports_t *reports = begin_reports(config.alloc);

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

static scan_t make_scanner_from_file(cthulhu_t *cthulhu, source_t *source)
{
    cerror_t error = 0;
    file_t handle = file_open(source->path, eFileRead | eFileText, &error);

    if (error != 0)
    {
        message_t *id = report(cthulhu->reports, eFatal, node_invalid(), "failed to open file `%s`", source->path);
        report_note(id, "%s", error_string(error));
        return scan_invalid();
    }

    return scan_file(cthulhu->reports, cthulhu->driver.name, handle);
}

static scan_t make_scanner_from_string(cthulhu_t *cthulhu, source_t *source)
{
    return scan_string(cthulhu->reports, cthulhu->driver.name, source->path, source->string);
}

static scan_t make_scanner_from_source(cthulhu_t *cthulhu, source_t *source)
{
    scan_t (*make[])(cthulhu_t *, source_t *) = {
        [eSourceFile] = make_scanner_from_file,
        [eSourceString] = make_scanner_from_string,
    };

    return make[source->kind](cthulhu, source);
}

status_t cthulhu_init(cthulhu_t *cthulhu)
{
    cthulhu->driver.fnInitCompiler(&cthulhu->runtime);

    size_t totalSources = total_sources(cthulhu);

    cthulhu->compiles = arena_malloc(get_alloc(cthulhu), totalSources * sizeof(compile_t), "compile-pipelines");

    for (size_t i = 0; i < totalSources; i++)
    {
        source_t *source = vector_get(cthulhu->sources, i);

        compile_t *ctx = get_compile(cthulhu, i);
        ctx->ast = NULL;
        ctx->hlir = NULL;
        ctx->source = source;
        ctx->scanner = make_scanner_from_source(cthulhu, source);
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
        source_t *source = vector_get(cthulhu->sources, i);
        compile_t *ctx = get_compile(cthulhu, i);
        hlir_t *hlir = ctx->hlir;

        rename_module(hlir, source->path);
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
