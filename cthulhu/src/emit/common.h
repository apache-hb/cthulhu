#pragma once

#include "cthulhu/emit/emit.h"
#include "cthulhu/ssa/ssa.h"

#include "io/io.h"

typedef struct names_t {
    size_t counter;
    map_t *names;
} names_t;

typedef struct emit_t {
    reports_t *reports;

    names_t block_names;
    names_t vreg_names;
} emit_t;

char *begin_module(emit_t *emit, fs_t *fs, const ssa_module_t *mod);
void end_module(emit_t *emit);

names_t names_new(size_t size);
void counter_reset(emit_t *emit);

char *get_step_name(emit_t *emit, const ssa_step_t *step);
char *get_block_name(emit_t *emit, const ssa_block_t *block);
char *get_step_from_block(emit_t *emit, const ssa_block_t *block, size_t index);

void write_string(io_t *io, const char *fmt, ...);

const char *type_to_string(const ssa_type_t *type);
