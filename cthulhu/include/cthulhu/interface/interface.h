#pragma once

#include "base/version-def.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/sema.h"
#include "platform/file.h"
#include "report/report.h"

BEGIN_API

typedef struct argparse_t argparse_t;

///
/// code required by interfaces
///

typedef struct
{
    reports_t *reports;
    map_t *modules;
    argparse_t *args;
} runtime_t;

typedef struct
{
    scan_t *scan;

    void *ast;
    const char *moduleName;
    hlir_t *hlir;
    sema_t *sema;
} compile_t;

// add commands to the command line parser
typedef void (*add_commands_t)(vector_t **);

// setup the compiler
typedef void (*init_compiler_t)(runtime_t *);

// parse and register a module
typedef void *(*parse_file_t)(runtime_t *, compile_t *);

// forward declare all exports
typedef void (*forward_decls_t)(runtime_t *, compile_t *);

// process all imports
typedef void (*import_modules_t)(runtime_t *, compile_t *);

// compile the file
typedef hlir_t *(*compile_module_t)(runtime_t *, compile_t *);

typedef struct
{
    const char *name;
    version_t version;

    // null terminated list of file extensions
    const char **exts;

    add_commands_t fnAddCommands;
    init_compiler_t fnInitCompiler;
    parse_file_t fnParseFile;
    forward_decls_t fnForwardDecls;
    import_modules_t fnResolveImports;
    compile_module_t fnCompileModule;
} driver_t;

typedef struct
{
    driver_t driver;
    report_config_t reportConfig;

    status_t status;

    reports_t *reports;

    runtime_t runtime;

    compile_t *compiles;

    vector_t *sources;
} cthulhu_t;

cthulhu_t *cthulhu_new(driver_t driver, vector_t *sources, argparse_t *args, reports_t *reports, report_config_t config);

typedef status_t (*cthulhu_step_t)(cthulhu_t *);

status_t cthulhu_init(cthulhu_t *cthulhu);
status_t cthulhu_parse(cthulhu_t *cthulhu);
status_t cthulhu_forward(cthulhu_t *cthulhu);
status_t cthulhu_resolve(cthulhu_t *cthulhu);
status_t cthulhu_compile(cthulhu_t *cthulhu);

vector_t *cthulhu_get_modules(cthulhu_t *cthulhu);

/**
 * @brief initialize the common runtime, always the first function an interface
 * should call
 */
void common_init(void);

// interface implemented by the language driver

driver_t get_driver(void);

END_API
