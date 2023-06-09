#pragma once

#include "report/report.h"

#include "cthulhu/mediator/mediator.h"

#include "cthulhu/hlir/check.h"

#define EXEC(mod, fn, ...) do { if (mod->fn != NULL) { logverbose("%s:" #fn "()", mod->id); mod->fn(__VA_ARGS__); } } while (0)

lang_handle_t *lang_new(lifetime_t *lifetime, const language_t *lang);
compile_t *lifetime_add_module(lifetime_t *self, lang_handle_t *handle, const char *name, compile_t *data);

compile_t *compile_init(lang_handle_t *handle, void *ast, sema_t *sema, hlir_t *mod);

void lang_compile(lang_handle_t *handle, compile_t *compile, check_t *check);
void lang_import(lang_handle_t *handle, compile_t *compile);

void mediator_configure(mediator_t *self, ap_t *ap);
