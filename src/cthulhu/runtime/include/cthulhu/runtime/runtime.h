#pragma once

#include <ctu_runtime_api.h>

#include "cthulhu/tree/context.h"

#include "core/analyze.h"
#include "core/version_def.h"
#include "notify/diagnostic.h"

CT_BEGIN_API

typedef struct mediator_t mediator_t;
typedef struct lifetime_t lifetime_t;
typedef struct driver_t driver_t;
typedef struct plugin_t plugin_t;
typedef struct context_t context_t;
typedef struct arena_t arena_t;
typedef struct map_t map_t;

typedef struct diagnostic_t diagnostic_t;
typedef struct vector_t vector_t;
typedef struct ap_t ap_t;
typedef struct tree_t tree_t;
typedef struct tree_cookie_t tree_cookie_t;
typedef struct scan_t scan_t;
typedef struct logger_t logger_t;
typedef struct cfg_group_t cfg_group_t;
typedef struct scan_callbacks_t scan_callbacks_t;

///
/// drivers
///

/// @ingroup mediator
/// @{

/// @brief initialize a language driver
///
/// @param driver the driver to initialize
typedef void (*driver_create_t)(driver_t *driver);

/// @brief destroy a language driver
///
/// @param driver the driver to destroy
typedef void (*driver_destroy_t)(driver_t *driver);

/// @brief parse a file into an AST
/// the ast should be exported via @a scan_set
///
/// @param driver the driver to use
/// @param scan the scanner to use
typedef void (*driver_parse_t)(driver_t *driver, scan_t *scan);

/// @brief return the context data needed for a scanner
///
/// @param driver the driver to use
/// @param scan the scanner to use
///
/// @return the context data
typedef void *(*driver_prepass_t)(driver_t *driver, scan_t *scan);

/// @brief run after a successful parse
///
/// @param driver the driver to use
/// @param scan the scanner to use
/// @param ast the language ast produced by the parser
typedef void (*driver_postpass_t)(driver_t *driver, scan_t *scan, void *ast);

/// @brief get the schema for the driver
///
/// @param driver the driver to get the schema of
/// @param root the group to add the schema to
///
/// @return the group for this driver
typedef cfg_group_t *(*driver_config_t)(driver_t *driver, cfg_group_t *root);

/// @brief a compile state
typedef enum compile_stage_t
{
#define STAGE(ID, STR) ID,
#include "mediator.def"

    eStageTotal
} compile_stage_t;

/// @brief a driver pass to be run on each translation unit
///
/// @param context the context that is being compiled
typedef void (*driver_pass_t)(context_t *context);

/// @brief common information about anything the mediator supports
typedef struct module_info_t
{
    /// @brief unique id for the module
    const char *id;

    /// @brief the human readable name for the module
    const char *name;

    /// @brief the version of the module
    version_info_t version;

    /// @brief all diagnostics associated with this module
    diagnostic_list_t diagnostics;
} module_info_t;

/// @brief a language driver support capabilities
typedef struct language_t
{
    /// @brief common information about the language
    module_info_t info;

    /// @brief the file extensions this language can parse
    /// @note this is a null terminated array
    const char * const *exts;

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
    const scan_callbacks_t *parse_callbacks;

    /// @brief an array of passes to run on each translation unit
    driver_pass_t fn_compile_passes[eStageTotal];
} language_t;

/// @brief plugin support capabilities
typedef struct plugin_t
{
    module_info_t info;
} plugin_t;

/// @brief compile a tree
typedef void (*target_tree_t)(tree_t *tree);

/// @brief compile ssa modules
typedef void (*target_module_t)(map_t *modules);

/// @brief a codegen target backend
typedef struct target_t
{
    module_info_t info;

    target_tree_t fn_tree;
    target_module_t fn_ssa;
} target_t;

/// @brief the frontend running the mediator
typedef struct frontend_t
{
    /// @brief information about the frontend
    module_info_t info;
} frontend_t;

/// @brief get the logger for a lifetime
///
/// @param lifetime the lifetime to get the logger of
///
/// @return the logger
CT_NODISCARD
CT_RUNTIME_API logger_t *lifetime_get_logger(IN_NOTNULL lifetime_t *lifetime);

/// @brief get the arena for a lifetime
///
/// @param lifetime the lifetime to get the arena of
///
/// @return the arena
CT_NODISCARD
CT_RUNTIME_API arena_t *lifetime_get_arena(IN_NOTNULL lifetime_t *lifetime);

/// @brief get the recursive resolution cookie for a lifetime
///
/// @param lifetime the lifetime to get the cookie of
///
/// @return the cookie
CT_NODISCARD
CT_RUNTIME_API tree_cookie_t *lifetime_get_cookie(IN_NOTNULL lifetime_t *lifetime);

/// @brief get the name of a compile stage
///
/// @param stage the stage to get the name of
///
/// @return the name of @p stage
CT_NODISCARD
CT_RUNTIME_API const char *stage_to_string(compile_stage_t stage);

/// @}

CT_END_API
