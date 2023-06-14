#pragma once

#include "base/version-def.h"

typedef struct mediator_t mediator_t;
typedef struct lifetime_t lifetime_t;
typedef struct driver_t driver_t;
typedef struct context_t context_t;

typedef struct hlir_t hlir_t;
typedef struct scan_t scan_t;
typedef struct reports_t reports_t;

typedef void (*create_t)(driver_t *);
typedef void (*destroy_t)(driver_t *);

typedef void (*parse_t)(driver_t *, scan_t *);

typedef enum compile_stage_t
{
#define STAGE(ID, STR) ID,
#include "cthulhu/mediator/mediator-def.inc"

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

reports_t *lifetime_get_reports(lifetime_t *lifetime);

const char *stage_to_string(compile_stage_t stage);
