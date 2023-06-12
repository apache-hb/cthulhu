#pragma once

#include "base/version-def.h"

typedef struct mediator_t mediator_t;
typedef struct lifetime_t lifetime_t;
typedef struct context_t context_t;

typedef struct hlir_t hlir_t;
typedef struct scan_t scan_t;

typedef void (*create_t)(mediator_t *);
typedef void (*destroy_t)(mediator_t *);

typedef void (*parse_t)(lifetime_t *, scan_t *);

// forward all symbols from the current context
typedef void (*forward_symbols_t)(context_t *);

// add symbols from imports to the current context
typedef void (*compile_imports_t)(context_t *);

// fully compile a symbol and all its dependencies
typedef void (*compile_symbol_t)(context_t *, hlir_t *);

typedef struct language_t 
{
    const char *id; ///< unique identifier for the language
    const char *name; ///< human readable name for the language

    version_info_t version; ///< version info for the frontend

    const char **exts; ///< null terminated list of file extensions

    create_t fnCreate; ///< called at startup
    destroy_t fnDestroy; ///< called at shutdown

    parse_t fnParse; ///< parse a file into an ast
    forward_symbols_t fnForwardSymbols; ///< forward all symbols from the current context
    compile_imports_t fnCompileImports; ///< add symbols from imports to the current context
    compile_symbol_t fnCompileSymbol; ///< fully compile a symbol and all its dependencies
} language_t;
