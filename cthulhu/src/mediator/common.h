#pragma once

#include "cthulhu/mediator/mediator.h"

#define EXEC(mod, fn, ...) do { if (mod->fn != NULL) { mod->fn(__VA_ARGS__); } } while (0)

lang_handle_t *lang_init(lifetime_t *lifetime, const language_t *lang);

void lang_compile(lang_handle_t *handle, hlir_t *mod);
