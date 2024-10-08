// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_broker_api.h>

#include "arena/arena.h"
#include "core/compiler.h"
#include "core/text.h"
#include "core/version_def.h"

#include "notify/diagnostic.h"

#include <stdint.h>

typedef struct scan_callbacks_t scan_callbacks_t;
typedef struct scan_t scan_t;
typedef struct arena_t arena_t;
typedef struct fs_t fs_t;
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

typedef struct target_emit_t target_emit_t;

#define CT_LANG_EXTS(...) { __VA_ARGS__, NULL }

/// @brief the name of a module
/// to represent the name `java.lang` use `CT_TEXT_VIEW("java\0lang")
typedef text_view_t unit_id_t;

typedef enum broker_pass_t
{
#define BROKER_PASS(ID, STR) ID,
#include "broker.inc"

    ePassCount
} broker_pass_t;

/// @brief stages of compilation
typedef enum broker_stage_t
{
#define BROKER_STAGE(ID, STR) ID,
#include "broker.inc"

    eStageCount
} broker_stage_t;

/// @brief plugin events generated by the broker
typedef enum broker_event_t
{
#define BROKER_EVENT(ID, STR) ID,
#include "broker.inc"

    eEventCount
} broker_event_t;

typedef enum broker_arena_t
{
#define BROKER_ARENA(ID, STR) ID,
#include "broker.inc"

    eArenaCount
} broker_arena_t;

/// @brief output folder structure
typedef enum file_layout_t
{
#define FILE_LAYOUT(ID, STR) ID,
#include "broker.inc"
    eFileLayoutCount
} file_layout_t;

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

typedef void (*language_preparse_t)(language_runtime_t *runtime, void *context);
typedef void (*language_postparse_t)(language_runtime_t *runtime, scan_t *scan, void *ast);

typedef struct language_builtins_t
{
    /// @brief the name of the builtin module
    unit_id_t name;

    /// @brief passthrough decl sizes for the builtin module
    STA_FIELD_SIZE(length) const size_t *decls;

    /// @brief name of each decl tag
    STA_FIELD_SIZE(length) const char * const *names;

    /// @brief size of decls
    STA_FIELD_RANGE(eSemaCount, SIZE_MAX) size_t length;
} language_info_t;

/// @brief convert a tree node to a string
typedef char *(*lang_repr_tree_t)(tree_t *tree, arena_t *arena);

/// @brief a language driver support capabilities
typedef struct language_t
{
    /// @brief common information about the language
    module_info_t info;

    /// @brief builtin module configuration
    language_info_t builtin;

    /// @brief convert a tree node to a string
    lang_repr_tree_t repr_tree;

    /// @brief the default file extensions this language should be used for
    /// @note this is a null terminated array
    const char * const *exts;

    /// @brief the size of the scan context for this language
    size_t context_size;

    /// @brief the size of an ast node for this language
    size_t ast_size;

    /// @brief called once at startup
    language_create_t fn_create;

    /// @brief called at shutdown
    language_destroy_t fn_destroy;

    /// @brief called before a file is parsed
    /// should return a pointer to a context that will be put into the scanner
    language_preparse_t fn_preparse;

    /// @brief called after a file is parsed
    /// should produce translation units from a scanned ast
    language_postparse_t fn_postparse;

    /// @brief callbacks for the parser
    const scan_callbacks_t *scanner;

    /// @brief an array of passes to run on each translation unit
    language_pass_t fn_passes[ePassCount];
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

typedef void (*plugin_create_t)(plugin_runtime_t *runtime);
typedef void (*plugin_destroy_t)(plugin_runtime_t *runtime);

/// @brief plugin support capabilities
typedef struct plugin_t
{
    /// @brief information about the plugin
    module_info_t info;

    /// @brief called once at startup
    plugin_create_t fn_create;

    /// @brief called at shutdown
    plugin_destroy_t fn_destroy;

    /// @brief the events this plugin is interested in
    event_list_t events;
} plugin_t;

typedef void (*target_create_t)(target_runtime_t *runtime);
typedef void (*target_destroy_t)(target_runtime_t *runtime);

/// @brief tree output generation
typedef void (*target_tree_t)(target_runtime_t *runtime, const tree_t *tree, target_emit_t *emit);

typedef struct emit_result_t
{
    vector_t *files;
} emit_result_t;

/// @brief ssa output generation
typedef emit_result_t (*target_ssa_t)(target_runtime_t *runtime, const ssa_result_t *ssa, target_emit_t *emit);

/// @brief a codegen target backend
typedef struct target_t
{
    /// @brief information about the target
    module_info_t info;

    /// @brief called once at startup
    target_create_t fn_create;

    /// @brief called at shutdown
    target_destroy_t fn_destroy;

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

///
/// runtimes
///

typedef struct language_runtime_t
{
    const language_t *info;
    broker_t *broker;

    /// @brief default memory arena
    arena_t *arena;

    /// @brief arena for the language ast
    arena_t *ast_arena;

    /// @brief logger
    logger_t *logger;

    /// @brief the builtins module for this language
    tree_t *root;
} language_runtime_t;

typedef struct compile_unit_t
{
    /// @brief the language that this originated from
    language_runtime_t *lang;

    /// @brief the ast for this unit
    /// is NULL if this is a builtin/precompiled unit
    void *ast;

    /// @brief the tree for this unit
    /// is NULL if the unit has not been forward declared yet
    tree_t *tree;
} compile_unit_t;

typedef struct plugin_runtime_t
{
    const plugin_t *info;
} plugin_runtime_t;

typedef struct target_runtime_t
{
    const target_t *info;
    broker_t *broker;

    arena_t *arena;

    logger_t *logger;
} target_runtime_t;

typedef struct target_emit_t
{
    file_layout_t layout;

    fs_t *fs;
} target_emit_t;

/// broker api
/// should only really be called by the frontend

RET_NOTNULL
CT_BROKER_API broker_t *broker_new(IN_NOTNULL const frontend_t *frontend, IN_NOTNULL arena_t *arena);

CT_BROKER_API language_runtime_t *broker_add_language(IN_NOTNULL broker_t *broker, IN_NOTNULL const language_t *lang);
CT_BROKER_API plugin_runtime_t *broker_add_plugin(IN_NOTNULL broker_t *broker, IN_NOTNULL const plugin_t *plugin);
CT_BROKER_API target_runtime_t *broker_add_target(IN_NOTNULL broker_t *broker, IN_NOTNULL const target_t *target);

CT_BROKER_API void broker_init(IN_NOTNULL broker_t *broker);
CT_BROKER_API void broker_deinit(IN_NOTNULL broker_t *broker);

CT_BROKER_API compile_unit_t *broker_get_unit(IN_NOTNULL broker_t *broker, unit_id_t id);

CT_BROKER_API void broker_parse(IN_NOTNULL language_runtime_t *runtime, IN_NOTNULL io_t *io);

CT_BROKER_API void broker_run_pass(IN_NOTNULL broker_t *broker, broker_pass_t pass);

CT_BROKER_API void broker_resolve(IN_NOTNULL broker_t *broker);

CT_BROKER_API logger_t *broker_get_logger(IN_NOTNULL broker_t *broker);
CT_BROKER_API const node_t *broker_get_node(IN_NOTNULL broker_t *broker);
CT_BROKER_API arena_t *broker_get_arena(IN_NOTNULL broker_t *broker);

/// @brief get all the modules in the broker
/// this does not include the root module
CT_BROKER_API vector_t *broker_get_modules(IN_NOTNULL broker_t *broker);

/// all runtime apis

CT_BROKER_API void lang_add_unit(IN_NOTNULL language_runtime_t *runtime, unit_id_t id, const node_t *node, void *ast, const size_t *sizes, size_t count);
CT_BROKER_API compile_unit_t *lang_get_unit(IN_NOTNULL language_runtime_t *runtime, unit_id_t id);

CT_BROKER_API void unit_update(IN_NOTNULL compile_unit_t *unit, IN_NOTNULL void *ast, IN_NOTNULL tree_t *tree);
CT_BROKER_API void *unit_get_ast(IN_NOTNULL compile_unit_t *unit);

CT_BROKER_API text_view_t build_unit_id(IN_NOTNULL const vector_t *parts, IN_NOTNULL arena_t *arena);

/// all plugin apis


/// all target apis

CT_BROKER_API void target_emit_tree(IN_NOTNULL target_runtime_t *runtime, IN_NOTNULL const tree_t *tree, IN_NOTNULL target_emit_t *emit);
CT_BROKER_API emit_result_t target_emit_ssa(IN_NOTNULL target_runtime_t *runtime, IN_NOTNULL const ssa_result_t *ssa, IN_NOTNULL target_emit_t *emit);

/// extra stuff

RET_NOTNULL CT_CONSTFN
CT_BROKER_API const char *broker_pass_name(IN_DOMAIN(<, ePassCount) broker_pass_t pass);

CT_CONSTFN CT_CONSTFN
CT_BROKER_API const char *file_layout_name(IN_DOMAIN(<, eFileLayoutCount) file_layout_t layout);

/// @}

CT_END_API
