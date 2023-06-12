#include "cthulhu/mediator/driver.h"

#include "std/vector.h"

#include "report/report.h"

static vector_t *example_lang_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "example");
    vector_push(&path, "lang");

    return path;
}

static void ex_create(lifetime_t *lifetime)
{
    vector_t *path = example_lang_path();
    context_t *ctx = context_new(lifetime, NULL, NULL);

    add_context(lifetime, path, ctx);

    logverbose("ex-create(0x%p)", lifetime);
}

static void ex_destroy(lifetime_t *lifetime)
{
    logverbose("ex-destroy(0x%p)", lifetime);
}

static void ex_parse(lifetime_t *lifetime, scan_t *scan)
{
    logverbose("ex-parse(0x%p, %s)", lifetime, scan_path(scan));
}

static void ex_forward(context_t *context)
{
    logverbose("ex-forward(0x%p)", context);
}

static void ex_import(context_t *context)
{
    logverbose("ex-import(0x%p)", context);
}

static void ex_compile(context_t *context, hlir_t *hlir)
{
    logverbose("ex-compile(0x%p, 0x%p)", context, hlir);
}

static const char *kLangNames[] = { "e", "example", NULL };

const language_t kExampleModule = {
    .id = "example",
    .name = "Example",
    .version = {
        .license = "GPLv3",
        .desc = "Example language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 0, 1)
    },

    .exts = kLangNames,
    
    .fnCreate = ex_create,
    .fnDestroy = ex_destroy,

    .fnParse = ex_parse,
    .fnForwardSymbols = ex_forward,
    .fnCompileImports = ex_import,
    .fnCompileSymbol = ex_compile
};
