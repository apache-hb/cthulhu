#pragma once

#include "ctu/util/io.h"
#include "ctu/util/report.h"
#include "ctu/lir/lir.h"
#include "ctu/gen/emit.h"

typedef scan_t*(*open_t)(reports_t*, file_t*);
typedef void*(*parse_t)(reports_t*, file_t*);
typedef lir_t*(*analyze_t)(reports_t*, void*);

typedef struct {
    const char *version;
    const char *name;
    open_t open;
    parse_t parse;
    analyze_t analyze;
} frontend_t;

typedef bool(*compile_t)(reports_t*, module_t*, const char*);

typedef struct {
    const char *version;
    const char *name;
    compile_t compile;
} backend_t;

extern const frontend_t FRONTEND_CTU;
extern const frontend_t FRONTEND_PL0;
extern const frontend_t FRONTEND_C11;

extern const backend_t BACKEND_LLVM;
extern const backend_t BACKEND_C99;
extern const backend_t BACKEND_GCCJIT;
extern const backend_t BACKEND_NULL;

/**
 * select a frontend based on a language name
 * 
 * @param reports report sink
 * @param name the language name to parse
 * 
 * @return the frontend or NULL if not found
 */
const frontend_t *select_frontend(reports_t *reports, const char *name);

/**
 * select a frontend for a file based on the file extension
 * 
 * @param reports report sink
 * @param frontend the default frontend from parse_args
 * @param path the file path
 * 
 * @return the frontend or NULL if not found
 */
const frontend_t *select_frontend_by_extension(reports_t *reports, const frontend_t *frotnend, const char *path);


/**
 * select a backend based on a target name
 * 
 * @param reports report sink
 * @param name the name of the backend
 * 
 * @return the backend or NULL if not found
 */
const backend_t *select_backend(reports_t *reports, const char *name);
