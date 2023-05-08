#include "cmd.h"

#include "cthulhu/mediator/mediator.h"
#include "std/vector.h"
#include "std/str.h"

#include "argparse/argparse.h"
#include "argparse/commands.h"

#include "io/io.h"

#include <stdio.h>

int main(int argc, const char **argv)
{
    runtime_init();
    mediator_t *mediator = mediator_new("cli", NEW_VERSION(0, 0, 1));

    mediator_region(mediator, eRegionLoadCompiler);

    runtime_t rt = cmd_parse(mediator, argc, argv);

    size_t errs = vector_len(rt.unknownArgs);
    for (size_t i = 0; i < errs; i++)
    {
        const char *err = vector_get(rt.unknownArgs, i);
        printf("error: %s\n", err);
    }

    mediator_region(mediator, eRegionInit);
    mediator_startup(mediator);

    mediator_region(mediator, eRegionLoadSource);

    size_t len = vector_len(rt.sourcePaths);
    vector_t *sources = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const char *path = vector_get(rt.sourcePaths, i);
        const char *ext = str_ext(path);
        if (ext == NULL)
        {
            printf("could not identify compiler for `%s` (no extension)\n", path);
            return 1;
        }

        lang_handle_t *handle = mediator_get_language_for_ext(mediator, ext);
        if (handle == NULL)
        {
            printf("could not identify compiler for `%s` (no language registered for extension `%s`)\n", path, ext);
            return 1;
        }

        io_t *io = io_file(path, eFileRead | eFileText);
        if (io_error(io) != 0)
        {
            printf("failed to load source `%s`\n%s\n", path, error_string(io_error(io)));
            return 1;
        }

        context_t *ctx = context_new(handle, io);
        
        vector_set(sources, i, ctx);
    }

    mediator_region(mediator, eRegionParse);

    for (size_t i = 0; i < len; i++)
    {
        context_t *ctx = vector_get(sources, i);
        mediator_parse(mediator, ctx);
    }

    mediator_region(mediator, eRegionCompile);

    for (size_t i = 0; i < len; i++)
    {
        context_t *ctx = vector_get(sources, i);
        mediator_compile(mediator, ctx);
    }

    vector_t *modules = vector_new(len);
    for (size_t i = 0; i < len; i++)
    {
        context_t *ctx = vector_get(sources, i);
        hlir_t *mod = get_context_module(ctx);

        vector_push(&modules, mod);
    }

    mediator_region(mediator, eRegionEnd);
    mediator_shutdown(mediator);
}
