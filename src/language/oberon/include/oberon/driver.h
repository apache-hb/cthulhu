// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "notify/diagnostic.h" // IWYU pragma: export

typedef struct language_runtime_t language_runtime_t;
typedef struct compile_unit_t compile_unit_t;
typedef struct tree_t tree_t;

#define NEW_EVENT(id, ...) CT_LOCAL extern const diagnostic_t kEvent_##id;
#include "oberon.inc"

void obr_create(language_runtime_t *runtime, tree_t *root);

void obr_forward_decls(language_runtime_t *runtime, compile_unit_t *unit);
void obr_process_imports(language_runtime_t *runtime, compile_unit_t *unit);
