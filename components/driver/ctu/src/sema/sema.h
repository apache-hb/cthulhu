#pragma once

#include "cthulhu/mediator/language.h"

#include "cthulhu/hlir/sema.h"
#include "cthulhu/hlir/digit.h"

typedef enum
{
    eTagAttribs = eSemaMax, // hlir_t*
    eTagSuffix,  // suffix_t*

    eTagTotal
} tag_t;

hlir_t *get_digit_type(sign_t sign, digit_t digit);

void ctu_init(lang_handle_t *runtime);
void ctu_forward_decls(lang_handle_t *runtime, const char *name, void *ast);
void ctu_process_imports(lang_handle_t *runtime, compile_t *compile);
hlir_t *ctu_compile_module(lang_handle_t *runtime, compile_t *compile);

sema_t *begin_sema(sema_t *parent, size_t *sizes);
void add_decl(sema_t *sema, int tag, const char *name, hlir_t *decl);
