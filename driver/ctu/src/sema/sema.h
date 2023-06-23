#pragma once

#include "cthulhu/mediator/driver.h"

#include "cthulhu/hlir/sema.h"
#include "cthulhu/hlir/digit.h"

typedef enum
{
    eTagAttribs = eSemaMax, // hlir_t*
    eTagSuffix,  // suffix_t*

    eTagTotal
} tag_t;

hlir_t *get_digit_type(sign_t sign, digit_t digit);

void ctu_init(driver_t *runtime);
void ctu_forward_decls(context_t *context);
void ctu_process_imports(context_t *context);
void ctu_compile_module(context_t *context);

sema_t *begin_sema(sema_t *parent, size_t *sizes);
void add_decl(sema_t *sema, int tag, const char *name, hlir_t *decl);
