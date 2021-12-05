#pragma once

#include "ctu/util/io.h"
#include "ctu/util/report.h"
#include "ctu/lir/lir.h"
#include "ctu/gen/emit.h"

struct settings_t;

typedef scan_t(*open_t)(reports_t*, file_t*);
typedef void*(*parse_t)(scan_t*);

/* return a vector of modules */
typedef vector_t*(*analyze_t)(reports_t *, struct settings_t*, void*);
typedef void(*init_t)(void);

typedef enum {
    ARG_BOOL,
    ARG_UINT,
    ARG_INT
} arg_type_t;

typedef struct {
    const char *name;
    const char *brief;
    const char *desc;
    arg_type_t type;
} arg_t;

arg_t *new_arg(
    arg_type_t type, 
    const char *name, 
    const char *brief, 
    const char *desc
);

typedef struct {
    const char *version;
    const char *name;
    init_t init;
    open_t open;
    parse_t parse;
    analyze_t analyze;
    vector_t *args; // map<string, arg_t>
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
