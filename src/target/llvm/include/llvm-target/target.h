// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "core/compiler.h"
#include "cthulhu/broker/broker.h"

typedef struct target_runtime_t target_runtime_t;
typedef struct tree_t tree_t;
typedef struct ssa_result_t ssa_result_t;
typedef struct target_emit_t target_emit_t;

CT_BEGIN_API

#define NEW_EVENT(id, ...) CT_LOCAL extern const diagnostic_t kEvent_##id;
#include "events.inc"

CT_LOCAL void llvm_create(target_runtime_t *runtime);
CT_LOCAL void llvm_destroy(target_runtime_t *runtime);

CT_LOCAL void llvm_tree(target_runtime_t *runtime, const tree_t *tree, target_emit_t *emit);
CT_LOCAL emit_result_t llvm_ssa(target_runtime_t *runtime, const ssa_result_t *ssa, target_emit_t *emit);

CT_END_API
