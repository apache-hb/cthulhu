// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "cthulhu/ssa/ssa.h"

typedef struct fs_t fs_t;

typedef struct names_t
{
    size_t counter;
    map_t *names;
} names_t;

typedef struct emit_t
{
    arena_t *arena;
    logger_t *reports;

    names_t block_names;
    names_t vreg_names;

    vector_t *files;
} emit_t;

char *begin_module(emit_t *emit, fs_t *fs, const ssa_module_t *mod);
void end_module(emit_t *emit);

names_t names_new(size_t size, arena_t *arena);
void counter_reset(emit_t *emit);

char *get_step_name(emit_t *emit, const ssa_step_t *step);
char *get_block_name(emit_t *emit, const ssa_block_t *block);
char *get_step_from_block(emit_t *emit, const ssa_block_t *block, size_t index);

const char *type_to_string(const ssa_type_t *type, arena_t *arena);
