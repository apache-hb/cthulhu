#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/version-def.h"

///
/// code required by interfaces
///

typedef struct
{
    reports_t *reports;
    map_t *modules;
} runtime_t;

typedef struct
{
    scan_t *scanner;
    void *ast;
    const char *moduleName;
    hlir_t *hlir;
    sema_t *sema;
} compile_t;

typedef void (*init_compiler_t)(runtime_t *);

// parse and register a module
typedef void (*parse_file_t)(runtime_t *, compile_t *);

// forward declare all exports
typedef void (*forward_decls_t)(runtime_t *, compile_t *);

// process all imports
typedef void (*import_modules_t)(runtime_t *, compile_t *);

// compile the file
typedef void (*compile_module_t)(runtime_t *, compile_t *);

typedef struct
{
    const char *name;
    version_t version;

    init_compiler_t fnInitCompiler;
    parse_file_t fnParseFile;
    forward_decls_t fnForwardDecls;
    import_modules_t fnResolveImports;
    compile_module_t fnCompileModule;
} driver_t;

// interface implemented by the language driver

CTHULHU_EXTERN driver_t get_driver(void);
