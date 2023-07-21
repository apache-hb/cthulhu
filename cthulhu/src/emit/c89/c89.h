#pragma once

#include "common.h"

typedef struct c89_emit_t {
    emit_t emit;

    map_t *modmap; // map<ssa_symbol, ssa_module>

    map_t *srcmap; // map<ssa_module, c89_source>
    map_t *hdrmap; // map<ssa_module, c89_source>

    fs_t *fs;
    map_t *deps;
    vector_t *sources;
} c89_emit_t;

const char *c89_format_type(c89_emit_t *emit, const ssa_type_t *type, const char *name);
const char *c89_format_params(c89_emit_t *emit, typevec_t *params, bool variadic);
