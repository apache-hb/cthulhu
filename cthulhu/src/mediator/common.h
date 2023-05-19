#pragma once

#include "cthulhu/mediator/mediator.h"

#define EXEC(mod, fn, ...) do { if (mod->fn != NULL) { mod->fn(__VA_ARGS__); } } while (0)

lang_handle_t *lang_init(lifetime_t *lifetime, const language_t *lang);
compile_t *lifetime_add_module(lifetime_t *self, lang_handle_t *handle, const char *name, compile_t *data);

compile_t *compile_init(lang_handle_t *handle, void *ast, sema_t *sema, hlir_t *mod);

void lang_compile(lang_handle_t *handle, compile_t *compile);
