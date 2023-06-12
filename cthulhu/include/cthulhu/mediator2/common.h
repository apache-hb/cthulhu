#pragma once

#include "base/version-def.h"

typedef struct mediator_t mediator_t;
typedef struct lifetime_t lifetime_t;
typedef struct context_t context_t;

typedef struct hlir_t hlir_t;

typedef void (*v2_init_t)(lifetime_t *);
typedef void (*v2_deinit_t)(lifetime_t *);

typedef void (*v2_parse_t)(lifetime_t *);

// forward all symbols from the current context
typedef void (*v2_forward_symbols_t)(context_t *);

// add symbols from imports to the current context
typedef void (*v2_compile_imports_t)(context_t *);

// fully compile a symbol and all its dependencies
typedef void (*v2_compile_symbol_t)(context_t *, hlir_t *);
