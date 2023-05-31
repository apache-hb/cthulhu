#pragma once

#include "mediator.h"

// language api

typedef void (*language_load_t)(mediator_t *);
typedef void (*language_unload_t)(mediator_t *);

typedef void (*language_configure_t)(lang_handle_t *, ap_t *);

typedef void (*language_init_t)(lang_handle_t *);
typedef void (*language_deinit_t)(lang_handle_t *);

// TODO: these should be structured differently, not quite sure how though

typedef void *(*language_parse_t)(lang_handle_t *, scan_t *);

typedef void (*language_forward_t)(lang_handle_t *, const char *, void *);

typedef void (*language_import_t)(lang_handle_t *, compile_t *);

typedef hlir_t *(*language_compile_t)(lang_handle_t *, compile_t *);

typedef struct language_t
{
    const char *id; ///< language driver id
    const char *name; ///< language driver name

    version_info_t version; ///< language driver version

    const char **exts; ///< null terminated list of default file extensions for this driver

    language_load_t fnLoad;
    language_unload_t fnUnload;

    language_configure_t fnConfigure; ///< configure the mediator to work with this driver
    
    language_init_t fnInit; ///< initialize the language driver
    language_deinit_t fnDeinit; ///< shutdown the language driver

    language_parse_t fnParse; ///< parse a source file
    language_forward_t fnForward; ///< forward declare all decls in a module
    language_import_t fnImport; ///< import all required modules
    language_compile_t fnCompile; ///< compile the ast to hlir
} language_t;

typedef const language_t *(*language_acquire_t)(mediator_t *);

// handle api

lang_handle_t *lifetime_get_lang_handle(lifetime_t *self, const language_t *it);

// driver api

sema_t *lang_find_module(lang_handle_t *self, const char *name);

void lang_set_user(lang_handle_t *self, void *user);

void *lang_get_user(lang_handle_t *self);

reports_t *lang_get_reports(lang_handle_t *self);

sema_t *handle_get_sema(lang_handle_t *self, const char *mod);

// compile api

void compile_begin(lang_handle_t *self, void *ast, const char *name, sema_t *sema, hlir_t *mod);

void compile_finish(compile_t *self);

void *compile_get_ast(compile_t *self);
sema_t *compile_get_sema(compile_t *self);
hlir_t *compile_get_module(compile_t *self);
