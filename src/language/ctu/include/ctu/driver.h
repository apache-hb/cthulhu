#pragma once

#include "notify/notify.h" // IWYU pragma: keep

typedef struct driver_t driver_t;
typedef struct context_t context_t;

void ctu_init(driver_t *handle);

void ctu_forward_decls(context_t *context);
void ctu_process_imports(context_t *context);
void ctu_compile_module(context_t *context);

#define NEW_EVENT(name, ...) extern const diagnostic_t kEvent_##name;
#include "events.def"
