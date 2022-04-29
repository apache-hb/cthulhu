#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/util/io.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/version-def.h"

#define CT_CALLBACKS(id, prefix)                                                                                       \
    static int prefix##_##id##_##init(scan_t *extra, void *scanner)                                                    \
    {                                                                                                                  \
        return prefix##lex_init_extra(extra, scanner);                                                                 \
    }                                                                                                                  \
    static void prefix##_##id##_set_in(FILE *fd, void *scanner)                                                        \
    {                                                                                                                  \
        prefix##set_in(fd, scanner);                                                                                   \
    }                                                                                                                  \
    static int prefix##_##id##_parse(scan_t *extra, void *scanner)                                                     \
    {                                                                                                                  \
        return prefix##parse(scanner, extra);                                                                          \
    }                                                                                                                  \
    static void *prefix##_##id##_scan(const char *text, void *scanner)                                                 \
    {                                                                                                                  \
        return prefix##_scan_string(text, scanner);                                                                    \
    }                                                                                                                  \
    static void prefix##_##id##_delete(void *buffer, void *scanner)                                                    \
    {                                                                                                                  \
        prefix##_delete_buffer(buffer, scanner);                                                                       \
    }                                                                                                                  \
    static void prefix##_##id##_destroy(void *scanner)                                                                 \
    {                                                                                                                  \
        prefix##lex_destroy(scanner);                                                                                  \
    }                                                                                                                  \
    static callbacks_t id = {                                                                                          \
        .init = prefix##_##id##_##init,                                                                                \
        .setIn = prefix##_##id##_set_in,                                                                               \
        .parse = prefix##_##id##_parse,                                                                                \
        .scan = prefix##_##id##_scan,                                                                                  \
        .destroyBuffer = prefix##_##id##_delete,                                                                       \
        .destroy = prefix##_##id##_destroy,                                                                            \
    }

typedef struct
{
    reports_t *reports;
    map_t *modules;
} runtime_t;

hlir_t *find_module(runtime_t *runtime, const char *path);

// TODO: how are we going to model circular imports?
// * we need to sanitize after each step, and all steps happen in order
//   this is going to require alot of memory barriers sadly
// - open all files and register all the modules at once
// - forward declare everything in all files at once
// - resolve all imports in all files at once
// - check for collisions
// - go through and compile each file

typedef struct
{
    scan_t *scanner;
    void *ast;
    const char *moduleName;
    hlir_t *hlirModule;
    sema_t *sema;
} compile_t;

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

    parse_file_t fnParseFile;
    forward_decls_t fnForwardDecls;
    import_modules_t fnResolveImports;
    compile_module_t fnCompileModule;
} driver_t;

/**
 * @brief initialize the common runtime, always the first function a driver
 * should call
 */
void common_init(void);

/**
 * @brief process the command line and run the compiler
 *
 * @param argc argc from main
 * @param argv argv from main
 * @param driver information about the driver being run
 * @return an exit code to return from main
 */
int common_main(int argc, const char **argv, driver_t driver);
