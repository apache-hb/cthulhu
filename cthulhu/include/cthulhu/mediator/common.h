#pragma once

#include "core/macros.h"
#include "core/version_def.h"

BEGIN_API

typedef struct mediator_t mediator_t;
typedef struct lifetime_t lifetime_t;
typedef struct driver_t driver_t;
typedef struct plugin_t plugin_t;
typedef struct context_t context_t;

typedef struct vector_t vector_t;
typedef struct ap_t ap_t;
typedef struct tree_t tree_t;
typedef struct cookie_t cookie_t;
typedef struct scan_t scan_t;
typedef struct reports_t reports_t;

///
/// drivers
///

typedef void (*config_t)(lifetime_t *, ap_t *);

typedef void (*driver_create_t)(driver_t *);
typedef void (*driver_destroy_t)(driver_t *);

typedef void (*parse_t)(driver_t *, scan_t *);

typedef enum compile_stage_t
{
#define STAGE(ID, STR) ID,
#include "cthulhu/mediator/mediator.inc"

    eStageTotal
} compile_stage_t;

typedef void (*driver_pass_t)(context_t *);

typedef struct language_t
{
    const char *id;   ///< unique identifier for the language
    const char *name; ///< human readable name for the language

    version_info_t version; ///< version info for the frontend

    const char **exts; ///< null terminated list of file extensions

    config_t fnConfig;

    driver_create_t fnCreate;   ///< called at startup
    driver_destroy_t fnDestroy; ///< called at shutdown

    parse_t fnParse; ///< parse a file into an ast

    driver_pass_t fnCompilePass[eStageTotal]; ///< compile a single pass
} language_t;

///
/// helpers
///

NODISCARD
reports_t *lifetime_get_reports(lifetime_t *lifetime);

NODISCARD
cookie_t *lifetime_get_cookie(lifetime_t *lifetime);

NODISCARD
const char *stage_to_string(compile_stage_t stage);

END_API
