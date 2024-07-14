// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "cthulhu/ssa/ssa.h"

typedef struct fs_t fs_t;

typedef struct names_t
{
    size_t counter;
    map_t *names;
} names_t;

typedef struct visit_ast_t
{
    arena_t *arena;
    logger_t *reports;

    names_t block_names;
    names_t vreg_names;
    names_t anon_names;
} emit_t;

CT_LOCAL char *begin_module(emit_t *emit, fs_t *fs, const ssa_module_t *mod);
CT_LOCAL void end_module(emit_t *emit);

CT_LOCAL names_t names_new(size_t size, arena_t *arena);
CT_LOCAL void counter_reset(emit_t *emit);

CT_LOCAL char *get_step_name(emit_t *emit, const ssa_step_t *step);
CT_LOCAL char *get_block_name(emit_t *emit, const ssa_block_t *block);
CT_LOCAL char *get_anon_symbol_name(emit_t *emit, const ssa_symbol_t *symbol, const char *prefix);
CT_LOCAL char *get_anon_local_name(emit_t *emit, const ssa_local_t *local, const char *prefix);
CT_LOCAL char *get_step_from_block(emit_t *emit, const ssa_block_t *block, size_t index);

CT_LOCAL const char *type_to_string(const ssa_type_t *type, arena_t *arena);
