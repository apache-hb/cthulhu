#pragma once

#include "notify/notify.h" // IWYU pragma: keep

typedef struct language_runtime_t language_runtime_t;
typedef struct compile_unit_t compile_unit_t;
typedef struct tree_t tree_t;

void ctu_init(language_runtime_t *runtime, tree_t *root);

void ctu_forward_decls(language_runtime_t *runtime, compile_unit_t *unit);
void ctu_process_imports(language_runtime_t *runtime, compile_unit_t *unit);
void ctu_compile_module(language_runtime_t *runtime, compile_unit_t *unit);

#define NEW_EVENT(name, ...) extern const diagnostic_t kEvent_##name;
#include "events.def"
