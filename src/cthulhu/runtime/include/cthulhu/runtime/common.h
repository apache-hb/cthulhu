#pragma once

#include "core/analyze.h"
#include "core/compiler.h"
#include "core/version_def.h"
#include "notify/diagnostic.h"

BEGIN_API

typedef struct mediator_t mediator_t;
typedef struct lifetime_t lifetime_t;
typedef struct driver_t driver_t;
typedef struct plugin_t plugin_t;
typedef struct context_t context_t;
typedef struct arena_t arena_t;

typedef struct diagnostic_t diagnostic_t;
typedef struct vector_t vector_t;
typedef struct ap_t ap_t;
typedef struct tree_t tree_t;
typedef struct cookie_t cookie_t;
typedef struct scan_t scan_t;
typedef struct logger_t logger_t;
typedef struct cfg_group_t cfg_group_t;
typedef struct callbacks_t callbacks_t;

///
/// drivers
///

typedef void (*driver_create_t)(driver_t *);
typedef void (*driver_destroy_t)(driver_t *);

typedef void (*driver_parse_t)(driver_t *, scan_t *);

/// @brief return the context data needed for a scanner
typedef void *(*driver_prepass_t)(driver_t *, scan_t *);

/// @brief get the context data from a scanner
typedef void (*driver_postpass_t)(driver_t *, scan_t *, void *);

/// @brief get the schema for the driver
typedef cfg_group_t *(*driver_config_t)(driver_t *, cfg_group_t *);

typedef enum compile_stage_t
{
#define STAGE(ID, STR) ID,
#include "mediator.inc"

    eStageTotal
} compile_stage_t;

typedef void (*driver_pass_t)(context_t *);

/// @brief a language drivers provided configuration
typedef struct language_t
{
    /// @brief the unique id for the language
    const char *id;

    /// @brief the human readable name for the language
    const char *name;

    /// @brief the version of the language
    version_info_t version;

    /// @brief the file extensions this language can parse
    /// @note this is a null terminated array
    const char * const *exts;

    /// @brief all diagnostics this language can produce
    diagnostic_list_t diagnostics;

    /// @brief get the schema for the driver
    driver_config_t fn_config;

    /// @brief called once at startup
    driver_create_t fn_create;

    /// @brief called at shutdown
    driver_destroy_t fn_destroy;

    /// @brief parse a file into an AST
    /// @note if @a parse_callbacks is set, this function is ignored
    driver_parse_t fn_parse;

    /// @brief called before parsing a file
    driver_prepass_t fn_preparse;

    /// @brief called after parsing a file
    driver_postpass_t fn_postparse;

    /// @brief callbacks for the parser
    const callbacks_t *parse_callbacks;

    /// @brief an array of passes to run on each translation unit
    driver_pass_t fn_compile_passes[eStageTotal];
} language_t;

///
/// helpers
///

NODISCARD
logger_t *lifetime_get_logger(lifetime_t *lifetime);

NODISCARD
arena_t *lifetime_get_arena(lifetime_t *lifetime);

NODISCARD
cookie_t *lifetime_get_cookie(lifetime_t *lifetime);

NODISCARD
const char *stage_to_string(compile_stage_t stage);

END_API
