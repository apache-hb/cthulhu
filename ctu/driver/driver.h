#pragma once

#include "ctu/util/io.h"
#include "ctu/util/report.h"
#include "ctu/lir/lir.h"
#include "ctu/gen/emit.h"

typedef scan_t(*open_t)(reports_t*, file_t*);
typedef void*(*parse_t)(scan_t *);

/* return a vector of modules */
typedef vector_t*(*analyze_t)(reports_t*, void*);
typedef void(*init_t)(void);

typedef struct {
    const char *version;
    const char *name;
    init_t init;
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

extern const backend_t BACKEND_LLVM;
extern const backend_t BACKEND_C99;
extern const backend_t BACKEND_GCCJIT;
extern const backend_t BACKEND_NULL;

/**
 * select a backend based on a target name
 * 
 * @param reports report sink
 * @param name the name of the backend
 * 
 * @return the backend or NULL if not found
 */
const backend_t *select_backend(reports_t *reports, const char *name);

/**
 * run the common main
 * 
 * @param frontend the frontend this is managing
 * @param argc the argc from main
 * @param argv the argv from main
 */
int common_main(const frontend_t *frontend, int argc, char **argv);
