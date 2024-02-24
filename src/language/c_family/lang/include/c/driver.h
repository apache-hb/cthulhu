// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "notify/notify.h" // IWYU pragma: keep

typedef struct language_runtime_t language_runtime_t;
typedef struct compile_unit_t compile_unit_t;
typedef struct tree_t tree_t;

void cc_create(language_runtime_t *runtime, tree_t *root);
void cc_destroy(language_runtime_t *runtime);

void cc_forward_decls(language_runtime_t *runtime, compile_unit_t *unit);
void cc_process_imports(language_runtime_t *runtime, compile_unit_t *unit);
void cc_compile_module(language_runtime_t *runtime, compile_unit_t *unit);

#define NEW_EVENT(name, ...) extern const diagnostic_t kEvent_##name;
#include "events.inc"
