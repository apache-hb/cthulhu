#pragma once

#include "base/version-def.h"
#include "base/macros.h"

/// driver <-> frontend interface mediator
/// 
/// every language provides a driver that implements compilation states
/// - init
///   - setup global data, called once only 
///
/// - parse source files
///   - parse a source file into an ast, called once per file
///
/// - forward declare decls
///   - forward declare all decls in a module, called once per ast
///
/// - imports required modules
///   - import all required modules, called once per ast
///
/// - compile to hlir
///   - compile the ast to hlir, called once per set of asts
/// 

/// every frontend provides an interface that implements the following
/// - context
///   - creates a context for the frontend
///
/// - parse config
///   - parses config of some description
///   - can be command line, config file, gui, etc
///
/// - run mediator
///   - runs interface mediator
///
///  - emit result
///    - emit the result of the compilation
///

/// the mediator is responsible for the following
/// - initializing all compilers
/// - initializing all plugins
///

BEGIN_API

typedef struct language_t language_t;
typedef struct plugin_t plugin_t;
typedef struct lifetime_t lifetime_t;
typedef struct mediator_t mediator_t;
typedef struct context_t context_t;
typedef struct reports_t reports_t;
typedef struct hlir_t hlir_t;
typedef struct sema_t sema_t;
typedef struct scan_t scan_t;
typedef struct ap_t ap_t;
typedef struct io_t io_t;

typedef enum region_t
{
    eRegionLoadCompiler, // load plugins and languages
    eRegionInit, // initialize everything
    eRegionLoadSource, // load source files
    eRegionParse, // parse source files
    eRegionCompile, // compile ast to hlir
    eRegionOptimize, // optimize hlir
    eRegionCodegen, // codegen hlir to ir
    eRegionCleanup, // cleanup everything
    eRegionEnd, // end of compilers lifetime

    eRegionTotal
} region_t;

typedef struct lang_handle_t
{
    mediator_t *mediator;
    const language_t *handle;

    void *user;
} lang_handle_t;

typedef struct plugin_handle_t
{
    mediator_t *mediator;
    const plugin_t *handle;

    void *user;
} plugin_handle_t;

// TODO: how should threading work? one thread per file or one thread per language?
// TODO: maybe it should be configurable, but then conflicts might happen

// TODO: should probably abstract argparse out
//       an intermediate config format that maps to toml/cmd/imgui would be nice

typedef void (*language_config_t)(lang_handle_t *, ap_t *);

typedef void (*language_init_t)(lang_handle_t *);

typedef void (*language_shutdown_t)(lang_handle_t *);

// TODO: these should be structured differently, not quite sure how though

typedef void *(*language_parse_t)(lang_handle_t *, context_t *);

typedef void (*language_forward_t)(lang_handle_t *, context_t *);

typedef void (*language_import_t)(lang_handle_t *, context_t *);

typedef hlir_t *(*language_compile_t)(lang_handle_t *, context_t *);

typedef struct language_t
{
    const char *id; ///< language driver id
    const char *name; ///< language driver name

    version_info_t version; ///< language driver version

    const char **exts; ///< null terminated list of default file extensions for this driver

    language_config_t fnConfigure; ///< configure the mediator to work with this driver
    
    language_init_t fnInit; ///< initialize the language driver
    language_shutdown_t fnShutdown; ///< shutdown the language driver

    language_parse_t fnParse; ///< parse a source file
    language_forward_t fnForward; ///< forward declare all decls in a module
    language_import_t fnImport; ///< import all required modules
    language_compile_t fnCompile; ///< compile the ast to hlir
} language_t;

typedef void (*plugin_config_t)(plugin_handle_t *, ap_t *);

typedef void (*plugin_init_t)(plugin_handle_t *);

typedef void (*plugin_shutdown_t)(plugin_handle_t *);

typedef void (*plugin_region_t)(plugin_handle_t *, region_t);

typedef struct plugin_t
{
    const char *id; ///< plugin id
    const char *name; ///< plugin name

    version_info_t version; ///< language driver version

    plugin_config_t fnConfigure; ///< configure the mediator to work with this plugin
    
    plugin_init_t fnInit; ///< initialize the plugin
    plugin_shutdown_t fnShutdown; ///< shutdown the plugin

    plugin_region_t fnRegion; ///< called when a region begins
} plugin_t;

void runtime_init(void);

// language api

typedef const language_t *(*language_load_t)(mediator_t *);

#define LANGUAGE_ENTRY_POINT language_load

#ifdef CC_MSVC
#   define LANGUAGE_EXPORT __declspec(dllexport)
#else
#   define LANGUAGE_EXPORT __attribute__((visibility("default")))
#endif

// plugin api

typedef const plugin_t *(*plugin_load_t)(mediator_t *);

#define PLUGIN_ENTRY_POINT plugin_load

#ifdef CC_MSVC
#   define PLUGIN_EXPORT __declspec(dllexport)
#else
#   define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

// context api

context_t *context_new(lang_handle_t *handle, io_t *io);

hlir_t *context_get_module(context_t *ctx);
scan_t *context_get_scanner(context_t *ctx);
reports_t *context_get_reports(context_t *ctx);
void *context_get_ast(context_t *ctx);
sema_t *context_get_sema(context_t *ctx);
const char *context_get_name(context_t *ctx);

void context_begin(context_t *ctx, sema_t *sema, hlir_t *mod);
void context_end(context_t *ctx, hlir_t *mod);

// handle api

sema_t *handle_get_sema(lang_handle_t *handle, const char *path);

lang_handle_t *lifetime_get_handle(lifetime_t *self, const language_t *it);

// lifetime api

lifetime_t *mediator_get_lifetime(mediator_t *self);

// mediator api

mediator_t *mediator_new(const char *name, version_t version);

void mediator_add_language(mediator_t *self, const language_t *language, ap_t *ap);
void mediator_add_plugin(mediator_t *self, const plugin_t *plugin, ap_t *ap);

const language_t *mediator_register_extension(mediator_t *self, const char *ext, const language_t *lang);

lang_handle_t *mediator_get_language(mediator_t *self, const char *id);
lang_handle_t *mediator_get_language_for_ext(mediator_t *self, const char *ext);

void mediator_parse(mediator_t *self, context_t *ctx);

void mediator_compile(mediator_t *self, context_t *ctx);

void mediator_region(mediator_t *self, region_t region);
void mediator_startup(mediator_t *self);
void mediator_shutdown(mediator_t *self);

END_API
