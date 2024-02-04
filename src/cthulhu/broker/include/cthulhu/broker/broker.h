#pragma once

#include <ctu_broker_api.h>

#include "core/compiler.h"
#include "core/text.h"
#include "core/version_def.h"

#include "notify/diagnostic.h"

typedef struct scan_callbacks_t scan_callbacks_t;
typedef struct scan_t scan_t;
typedef struct arena_t arena_t;
typedef struct tree_t tree_t;
typedef struct vector_t vector_t;
typedef struct tree_cookie_t tree_cookie_t;
typedef struct io_t io_t;
typedef struct node_t node_t;
typedef struct logger_t logger_t;
typedef struct tree_attrib_t tree_attrib_t;
typedef struct ssa_result_t ssa_result_t;

CT_BEGIN_API

/// @defgroup broker Compiler runtime mediator
/// @brief Core of the compiler, manages languages, plugins, and targets
/// @ingroup runtime
/// @{

typedef struct frontend_t frontend_t;
typedef struct language_t language_t;
typedef struct plugin_t plugin_t;
typedef struct target_t target_t;
typedef struct broker_t broker_t;

typedef struct language_runtime_t language_runtime_t;
typedef struct compile_unit_t compile_unit_t;
typedef struct plugin_runtime_t plugin_runtime_t;
typedef struct target_runtime_t target_runtime_t;

/// @brief stages of compilation
typedef enum broker_stage_t
{
#define BROKER_STAGE(ID, STR) ID,
#include "broker.def"

    eStageCount
} broker_stage_t;

/// @brief plugin events generated by the broker
typedef enum broker_event_t
{
#define BROKER_EVENT(ID, STR) ID,
#include "broker.def"

    eEventCount
} broker_event_t;

/// @brief common information about anything the broker supports
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

/// @brief a language compilation pass
typedef void (*language_pass_t)(language_runtime_t *runtime, compile_unit_t *unit);

/// @brief initialize the root module
typedef void (*language_create_t)(language_runtime_t *runtime, tree_t *root);
typedef void (*language_destroy_t)(language_runtime_t *runtime);

typedef void *(*language_preparse_t)(language_runtime_t *runtime);
typedef void (*language_postparse_t)(language_runtime_t *runtime, scan_t *scan, void *ast);

typedef struct language_builtins_t
{
    text_view_t name;
    const size_t *decls;
    size_t length;
} language_builtins_t;

/// @brief a language driver support capabilities
typedef struct language_t
{
    /// @brief common information about the language
    module_info_t info;

    /// @brief builtin module configuration
    language_builtins_t builtin;

    /// @brief the file extensions this language can parse
    /// @note this is a null terminated array
    const char * const *exts;

    language_create_t fn_create;
    language_destroy_t fn_destroy;

    language_preparse_t fn_preparse;
    language_postparse_t fn_postparse;

    /// @brief callbacks for the parser
    const scan_callbacks_t *scanner;

    /// @brief an array of passes to run on each translation unit
    language_pass_t fn_passes[eStageCount];
} language_t;

/// @brief a plugin event callback description
typedef struct plugin_event_t
{
    /// @brief the event to listen for
    broker_event_t event;
} plugin_event_t;

typedef struct event_list_t
{
    const plugin_event_t *events;
    size_t count;
} event_list_t;

/// @brief plugin support capabilities
typedef struct plugin_t
{
    /// @brief information about the plugin
    module_info_t info;

    /// @brief the events this plugin is interested in
    event_list_t events;
} plugin_t;

/// @brief tree output generation
typedef void (*target_tree_t)(target_runtime_t *runtime, tree_t *tree);

/// @brief ssa output generation
typedef void (*target_ssa_t)(target_runtime_t *runtime, ssa_result_t *ssa);

/// @brief a codegen target backend
typedef struct target_t
{
    /// @brief information about the target
    module_info_t info;

    /// @brief generate from the tree form
    target_tree_t fn_tree;

    /// @brief generate from the ssa form
    target_ssa_t fn_ssa;
} target_t;

/// @brief the frontend running the mediator
typedef struct frontend_t
{
    /// @brief information about the frontend
    module_info_t info;
} frontend_t;

/// broker api
/// should only really be called by the frontend

RET_NOTNULL
CT_BROKER_API broker_t *broker_new(IN_NOTNULL const frontend_t *frontend, IN_NOTNULL arena_t *arena);

CT_BROKER_API void broker_add_languages(IN_NOTNULL broker_t *broker, IN_NOTNULL const language_t *lang, size_t count);
CT_BROKER_API void broker_add_plugins(IN_NOTNULL broker_t *broker, IN_NOTNULL const plugin_t *plugin, size_t count);
CT_BROKER_API void broker_add_targets(IN_NOTNULL broker_t *broker, IN_NOTNULL const target_t *target, size_t count);

CT_BROKER_API void broker_create_modules(IN_NOTNULL broker_t *broker);
CT_BROKER_API void broker_destroy_modules(IN_NOTNULL broker_t *broker);

CT_BROKER_API void broker_parse(IN_NOTNULL broker_t *broker, IN_NOTNULL const char *ext, IN_NOTNULL io_t *io);

CT_BROKER_API void broker_run_pass(IN_NOTNULL broker_t *broker, broker_stage_t stage);

CT_BROKER_API void broker_resolve(IN_NOTNULL broker_t *broker);

/// all runtime apis

CT_BROKER_API logger_t *lang_get_logger(IN_NOTNULL language_runtime_t *runtime);
CT_BROKER_API arena_t *lang_get_arena(IN_NOTNULL language_runtime_t *runtime);
CT_BROKER_API tree_t *lang_get_root(IN_NOTNULL language_runtime_t *runtime);
CT_BROKER_API const node_t *lang_get_node(IN_NOTNULL language_runtime_t *runtime);
CT_BROKER_API tree_cookie_t *lang_get_cookie(IN_NOTNULL language_runtime_t *runtime);

CT_BROKER_API compile_unit_t *lang_new_unit(IN_NOTNULL language_runtime_t *runtime, const char *name, void *ast, tree_t *tree);
CT_BROKER_API compile_unit_t *lang_new_compiled(IN_NOTNULL language_runtime_t *runtime, tree_t *tree);
CT_BROKER_API void lang_add_unit(IN_NOTNULL language_runtime_t *runtime, const vector_t *path, IN_NOTNULL compile_unit_t *unit);
CT_BROKER_API compile_unit_t *lang_get_unit(IN_NOTNULL language_runtime_t *runtime, IN_NOTNULL const vector_t *path);

CT_BROKER_API void unit_update(IN_NOTNULL compile_unit_t *unit, void *ast, tree_t *tree);
CT_BROKER_API void *unit_get_ast(IN_NOTNULL compile_unit_t *unit);
CT_BROKER_API tree_t *unit_get_tree(IN_NOTNULL compile_unit_t *unit);
CT_BROKER_API const char *unit_get_name(IN_NOTNULL compile_unit_t *unit);

/// all plugin apis

CT_BROKER_API logger_t *plugin_get_logger(IN_NOTNULL plugin_runtime_t *runtime);
CT_BROKER_API arena_t *plugin_get_arena(IN_NOTNULL plugin_runtime_t *runtime);

/// all target apis

CT_BROKER_API logger_t *target_get_logger(IN_NOTNULL target_runtime_t *runtime);
CT_BROKER_API arena_t *target_get_arena(IN_NOTNULL target_runtime_t *runtime);

/// @}

CT_END_API
