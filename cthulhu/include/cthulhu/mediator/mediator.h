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

typedef struct mediator_t mediator_t;
typedef struct lifetime_t lifetime_t;

typedef struct lang_handle_t lang_handle_t;
typedef struct plugin_handle_t plugin_handle_t;

typedef struct language_t language_t;
typedef struct plugin_t plugin_t;

typedef struct reports_t reports_t;
typedef struct hlir_t hlir_t;
typedef struct sema_t sema_t;
typedef struct scan_t scan_t;
typedef struct ap_t ap_t;
typedef struct io_t io_t;

// TODO: how should threading work? one thread per file or one thread per language?
// TODO: maybe it should be configurable, but then conflicts might happen

// TODO: replace xxx_config_t with xxx_load_t and xxx_unload_t
// TODO: should probably abstract argparse out
//       an intermediate config format that maps to toml/cmd/imgui would be nice

/// language lifetime flow

/// - load: called once at first load, setup globals

///   - configure: called once per lifetime, provide configuration info
///   - init: called once all languages have been configured. setup globals that depend on configuration

///     - parse: called once per file, parse a source file into an ast
///     - forward: called once per ast, forward declare all decls in a module
///     - import: called once per ast, import all required modules
///     - compile: called once per set of asts, compile the ast to hlir

///   - deinit: called once all languages have been deinitialized. cleanup globals that depend on configuration

/// - unload: called once at the end of the program, cleanup globals
///           will only be called if load was called

void runtime_init(void);

typedef struct source_t
{
    io_t *io;
    const language_t *lang;
} source_t;

// lifetime api

lifetime_t *mediator_get_lifetime(mediator_t *self);

/**
 * @brief add a module to the handle
 * 
 * @param handle 
 * @param name 
 * @param data 
 * @return void* NULL if the module was added, otherwise the module that already exists wih that name
 */
void *lifetime_add_module(lifetime_t *self, const char *name, void *data);

void *lifetime_get_module(const lifetime_t *self, const char *name);

void lifetime_add_source(lifetime_t *self, source_t source);

void lifetime_init(lifetime_t *self);
void lifetime_deinit(lifetime_t *self);

void lifetime_parse(reports_t *reports, lifetime_t *self);

// mediator api

mediator_t *mediator_new(const char *name, version_t version);

void mediator_load_language(mediator_t *self, const language_t *language);
void mediator_load_plugin(mediator_t *self, const plugin_t *plugin);

void mediator_unload_language(mediator_t *self, const language_t *language);
void mediator_unload_plugin(mediator_t *self, const plugin_t *plugin);

/**
 * @brief map an extension to a language
 * 
 * @param self the mediator
 * @param ext the extension
 * @param lang the language
 * @return const language_t* NULL if the extension is correctly registered, otherwise the language that already has the extension
 */
const language_t *mediator_register_extension(mediator_t *self, const char *ext, const language_t *lang);

const language_t *mediator_get_language(mediator_t *self, const char *id);
const language_t *mediator_get_language_by_ext(mediator_t *self, const char *ext);

END_API
