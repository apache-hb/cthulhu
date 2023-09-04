#pragma once

#include "common.h"

typedef struct c89_source_t {
    io_t *io;
    const char *path;
} c89_source_t;

typedef struct c89_emit_t {
    emit_t emit;

    map_t *modmap; // map<ssa_symbol, ssa_module>

    map_t *srcmap; // map<ssa_module, c89_source>
    map_t *hdrmap; // map<ssa_module, c89_source>

    const ssa_symbol_t *current;

    map_t *stepmap; // map<ssa_step, c89_source>
    map_t *strmap; // map<const char*, const char*>

    fs_t *fs;
    map_t *deps;
    vector_t *sources;
} c89_emit_t;

///
/// type formatting
///

const char *c89_format_type(c89_emit_t *emit, const ssa_type_t *type, const char *name, bool emitConst);
const char *c89_format_params(c89_emit_t *emit, typevec_t *params, bool variadic);

const char *c89_format_storage(c89_emit_t *emit, ssa_storage_t storage, const char *name);

///
/// symbol foward declarations
///

void c89_proto_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol);
void c89_proto_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol);

///
/// symbol definitions
///

void c89_define_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol);
void c89_define_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol);
