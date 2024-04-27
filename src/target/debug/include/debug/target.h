// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "core/compiler.h"
#include "cthulhu/broker/broker.h"

typedef struct target_runtime_t target_runtime_t;
typedef struct ssa_result_t ssa_result_t;
typedef struct target_emit_t target_emit_t;
typedef struct tree_t tree_t;

CT_BEGIN_API

CT_LOCAL emit_result_t debug_ssa(target_runtime_t *runtime, const ssa_result_t *ssa, target_emit_t *emit);
CT_LOCAL void debug_tree(target_runtime_t *runtime, const tree_t *tree, target_emit_t *emit);

CT_END_API
