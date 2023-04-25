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

typedef struct instance_t instance_t;
typedef struct mediator_t mediator_t;

typedef struct language_t
{
    const char *name; ///< language driver name
    version_t version; ///< driver version

    const char **exts; ///< null terminated list of default file extensions for this driver

    
} language_t;

void runtime_init(void);

END_API
