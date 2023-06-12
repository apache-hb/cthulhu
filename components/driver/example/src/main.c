#include "cthulhu/mediator/driver.h"

#include <stdio.h>

static void ex_create(mediator_t *mediator)
{
    printf("ex-load(%p)\n", mediator);
}

static void ex_destroy(mediator_t *mediator)
{
    printf("ex-unload(%p)\n", mediator);
}

static void ex_parse(lifetime_t *lifetime, scan_t *scan)
{
    printf("ex-parse(%p, %p)\n", lifetime, scan);
}

static void ex_forward(context_t *context)
{
    printf("ex-forward(%p)\n", context);
}

static void ex_import(context_t *context)
{
    printf("ex-import(%p)\n", context);
}

static void ex_compile(context_t *context, hlir_t *hlir)
{
    printf("ex-compile(%p, %p)\n", context, hlir);
}

static const char *kLangNames[] = { "e", "example", NULL };

const language_t kExampleModule = {
    .id = "example",
    .name = "Example",
    .version = {
        .license = "GPLv3",
        .desc = "Example language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 0, 0)
    },

    .exts = kLangNames,
    
    .fnCreate = ex_create,
    .fnDestroy = ex_destroy,

    .fnParse = ex_parse,
    .fnForwardSymbols = ex_forward,
    .fnCompileImports = ex_import,
    .fnCompileSymbol = ex_compile
};
