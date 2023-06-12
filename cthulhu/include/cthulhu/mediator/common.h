#pragma once

#include "base/version-def.h"

typedef struct mediator_t mediator_t;
typedef struct lifetime_t lifetime_t;
typedef struct context_t context_t;

typedef struct hlir_t hlir_t;
typedef struct scan_t scan_t;

typedef void (*create_t)(lifetime_t *);
typedef void (*destroy_t)(lifetime_t *);

typedef void (*parse_t)(lifetime_t *, scan_t *);

typedef enum compile_stage_t
{
    eStageForwardSymbols,
    eStageCompileImports,
    eStageCompileTypes,
    eStageCompileSymbols,

    eStageTotal
} compile_stage_t;

typedef void (*compile_pass_t)(context_t *);

typedef struct language_t 
{
    const char *id; ///< unique identifier for the language
    const char *name; ///< human readable name for the language

    version_info_t version; ///< version info for the frontend

    const char **exts; ///< null terminated list of file extensions

    create_t fnCreate; ///< called at startup
    destroy_t fnDestroy; ///< called at shutdown

    parse_t fnParse; ///< parse a file into an ast

    compile_pass_t fnCompilePass[eStageTotal]; ///< compile a single pass
} language_t;
