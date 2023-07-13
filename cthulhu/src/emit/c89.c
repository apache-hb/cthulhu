#include "common.h"

#include "std/vector.h"

typedef struct c89_emit_t {
    emit_t emit;

    fs_t *fs;
    map_t *deps;
    vector_t *sources;
} c89_emit_t;

static void emit_c89_module(c89_emit_t *emit, const ssa_module_t *mod)
{

}

c89_emit_result_t emit_c89(const c89_emit_options_t *options)
{
    emit_options_t opts = options->opts;
    c89_emit_t emit = {
        .emit = {
            .reports = opts.reports,
            .blockNames = names_new(64),
            .vregNames = names_new(64),
        },
        .sources = vector_new(32)
    };

    size_t len = vector_len(opts.modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        emit_c89_module(&emit, mod);
    }

    c89_emit_result_t result = {
        .sources = emit.sources,
    };
    return result;
}
