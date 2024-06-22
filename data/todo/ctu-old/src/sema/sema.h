#pragma once

#include "cthulhu/mediator/driver.h"

#include "cthulhu/hlir/h2.h"

typedef enum
{
    eTagValues = eSema2Values,
    eTagTypes = eSema2Types,
    eTagProcs = eSema2Procs,
    eTagModules = eSema2Modules,

    eTagAttribs, // h2_t*
    eTagSuffix,  // suffix_t*

    eTagTotal
} tag_t;

h2_t *get_digit_type(sign_t sign, digit_t digit);

void ctu_init(driver_t *runtime);
void ctu_forward_decls(context_t *context);
void ctu_process_imports(context_t *context);
void ctu_compile_module(context_t *context);

h2_t *begin_sema(h2_t *parent, size_t *sizes);
void add_decl(h2_t *sema, int tag, const char *name, h2_t *decl);
