#include "cthulhu/mediator/language.h"
#include "cthulhu/mediator/mediator.h"

#include <stdio.h>

static const char *kLangNames[] = { "e", "example", NULL };

static void ex_load(mediator_t *mediator)
{
    printf("ex-load(%p)\n", mediator);
}

static void ex_unload(mediator_t *mediator)
{
    printf("ex-unload(%p)\n", mediator);
}

static void ex_init(lang_handle_t *handle)
{
    printf("ex-init(%p)\n", handle);
}

static void ex_deinit(lang_handle_t *handle)
{
    printf("ex-deinit(%p)\n", handle);
}

static void *ex_parse(lang_handle_t *handle, scan_t *scan)
{
    printf("ex-parse(%p, %p)\n", handle, scan);

    return NULL;
}

static void ex_forward(lang_handle_t *handle, const char *name, void *ast)
{
    printf("ex-forward(%p, %s, %p)\n", handle, name, ast);
}

static hlir_t *ex_compile(lang_handle_t *handle, compile_t *compile)
{
    printf("ex-compile(%p, %p)\n", handle, compile);

    return NULL;
}

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
    
    .fnLoad = ex_load,
    .fnUnload = ex_unload,

    .fnInit = ex_init,
    .fnDeinit = ex_deinit,

    .fnParse = ex_parse,
    .fnForward = ex_forward,
    .fnCompile = ex_compile
};
