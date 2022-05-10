#include "cthulhu/ast/compile.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/interface/interface.h"
#include "cthulhu/interface/runtime.h"
#include "cthulhu/util/report.h"

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
    runtime_t runtime = {.reports = reports, .modules = map_optimal(size)};

    return runtime;
}

static source_t *source_of_kind(source_kind_t kind, const char *path)
{
    source_t *source = ctu_malloc(sizeof(source_t));
    source->kind = kind;
    source->path = path;
    return source;
}

source_t *source_file(const char *path)
{
    return source_of_kind(SOURCE_FILE, path);
}

source_t *source_string(const char *path, const char *string)
{
    source_t *source = source_of_kind(SOURCE_STRING, path);
    source->string = string;
    return source;
}

cthulhu_t *cthulhu_new(driver_t driver, vector_t *sources, config_t config)
{
    CTASSERT(driver.fnInitCompiler != NULL, "driver must implement fnInitCompiler");
    CTASSERT(driver.fnParseFile != NULL, "driver must implement fnParseFile");
    CTASSERT(driver.fnForwardDecls != NULL, "driver must implement fnForwardDecls");
    CTASSERT(driver.fnResolveImports != NULL, "driver must implement fnResolveImports");
    CTASSERT(driver.fnCompileModule != NULL, "driver must implement fnCompileModule");
    
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

static scan_t *make_scanner_from_file(cthulhu_t *cthulhu, source_t *source)
{
    cerror_t error = 0;
    file_t handle = file_open(source->path, 0, &error);

    if (error != 0)
    {
        message_t *id = report(cthulhu->reports, ERROR, NULL, "failed to open file `%s`", source->path);
        report_note(id, "%s", error_string(error));
        return NULL;
    }

    scan_t scanner = scan_file(cthulhu->reports, cthulhu->driver.name, handle);
    return BOX(scanner);
}

static scan_t *make_scanner_from_string(cthulhu_t *cthulhu, source_t *source)
{
    scan_t scanner = scan_string(cthulhu->reports, cthulhu->driver.name, source->path, source->string);
    return BOX(scanner);
}

static scan_t *make_scanner_from_source(cthulhu_t *cthulhu, source_t *source)
{
    scan_t *(*make[])(cthulhu_t *, source_t *) = {
        [SOURCE_FILE] = make_scanner_from_file, [SOURCE_STRING] = make_scanner_from_string,};

    return make[source->kind](cthulhu, source);
}

int cthulhu_init(cthulhu_t *cthulhu)
{
    cthulhu->driver.fnInitCompiler(&cthulhu->runtime);

    size_t totalSources = vector_len(cthulhu->sources);

    cthulhu->compiles = vector_of(totalSources);

    for (size_t i = 0; i < totalSources; i++)
    {
        source_t *source = vector_get(cthulhu->sources, i);

        compile_t *ctx = ctu_malloc(sizeof(compile_t));
        ctx->ast = NULL;
        ctx->hlir = NULL;
        ctx->source = source;
        ctx->scanner = make_scanner_from_source(cthulhu, source);

        vector_set(cthulhu->compiles, i, ctx);
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
