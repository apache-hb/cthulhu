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
typedef struct instance_t instance_t;
typedef struct mediator_t mediator_t;
typedef struct ap_t ap_t;

// TODO: how should threading work? one thread per file or one thread per language?
// TODO: maybe it should be configurable, but then conflicts might happen

// TODO: should probably abstract argparse out
//       an intermediate config format that maps to toml/cmd/imgui would be nice

typedef void (*language_config_t)(instance_t *);

typedef void (*language_init_t)(instance_t *);


// TODO: these should be structured differently, not quite sure how though

typedef void (*language_parse_t)(instance_t *);

typedef void (*language_forward_t)(instance_t *);

typedef void (*language_import_t)(instance_t *);

typedef void (*language_compile_t)(instance_t *);

typedef struct language_t
{
    const char *name; ///< language driver name
    version_t version; ///< driver version

    const char **exts; ///< null terminated list of default file extensions for this driver

    language_config_t fnConfigure; ///< configure the mediator to work with this driver
    language_init_t fnInit; ///< initialize the language driver

    language_parse_t fnParse; ///< parse a source file
    language_forward_t fnForward; ///< forward declare all decls in a module
    language_import_t fnImport; ///< import all required modules
    language_compile_t fnCompile; ///< compile the ast to hlir
} language_t;

typedef void (*plugin_config_t)(mediator_t *);

typedef void (*plugin_init_t)(mediator_t *);

typedef struct plugin_t
{
    const char *name; ///< plugin name
    version_t version; ///< plugin version

    plugin_config_t fnConfigure; ///< configure the mediator to work with this plugin
    plugin_init_t fnInit; ///< initialize the plugin
} plugin_t;

void runtime_init(void);

// language api

typedef const language_t *(*language_load_t)(mediator_t *);

#define LANGUAGE_ENTRY_POINT language_load

// plugin api

typedef const plugin_t *(*plugin_load_t)(mediator_t *);

#define PLUGIN_ENTRY_POINT plugin_load

// mediator api

mediator_t *mediator_new(const char *name, version_t version);

void mediator_add_language(mediator_t *self, const language_t *language);
void mediator_add_plugin(mediator_t *self, const plugin_t *plugin);

END_API
