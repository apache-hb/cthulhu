// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "common.h"

typedef struct io_t io_t;

typedef struct c89_source_t
{
    io_t *io;
    const char *path;
} c89_source_t;

typedef struct c89_emit_t
{
    arena_t *arena;

    emit_t emit;

    map_t *modmap; // map<ssa_symbol, ssa_module>

    map_t *srcmap; // map<ssa_module, c89_source>
    map_t *hdrmap; // map<ssa_module, c89_source>

    const ssa_symbol_t *current;

    map_t *stepmap; // map<ssa_step, c89_source>

    set_t *defined; // set<ssa_type>

    fs_t *fs;
    map_t *deps;
    vector_t *sources;
} c89_emit_t;

///
/// type formatting
///

typedef enum type_format_t {
    eFormatEmitNone = 0,
    eFormatEmitConst = 1 << 0,
} type_format_t;

const char *c89_format_type(c89_emit_t *emit, const ssa_type_t *type, const char *name, type_format_t flags);
const char *c89_format_params(c89_emit_t *emit, typevec_t *params, bool variadic);
const char *c89_printf_specifier(digit_t digit, sign_t sign);

const char *c89_format_storage(c89_emit_t *emit, ssa_storage_t storage, const char *name);

c89_source_t *c89_get_source(c89_emit_t *emit, const ssa_module_t *mod);
c89_source_t *c89_get_header(c89_emit_t *emit, const ssa_module_t *mod);

io_t *c89_get_header_io(c89_emit_t *emit, const ssa_module_t *mod);
io_t *c89_get_source_io(c89_emit_t *emit, const ssa_module_t *mod);

///
/// symbol foward declarations
///

void c89_proto_type(c89_emit_t *emit, io_t *io, const ssa_type_t *type);
void c89_proto_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol);
void c89_proto_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol);

///
/// symbol definitions
///

void c89_define_type(c89_emit_t *emit, io_t *io, const ssa_type_t *type);
void c89_define_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol);
void c89_define_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol);
