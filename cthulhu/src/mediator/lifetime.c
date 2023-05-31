#include "cthulhu/mediator/mediator.h"

#include "cthulhu/mediator/language.h"

#include "scan/compile.h"

#include "std/vector.h"
#include "std/set.h"
#include "std/map.h"
#include "std/str.h"

#include "io/io.h"

#include "base/util.h"
#include "base/panic.h"
#include "base/memory.h"

#include "common.h"

typedef struct lifetime_t
{
    mediator_t *parent;

    reports_t *reports;

    set_t *languages;
    map_t *handles; // map_t<const language_t *, lang_handle_t *>

    vector_t *files; /// vector_t<source_t>

    map_t *modules; // map_t<const char *, hlir_t *>
} lifetime_t;

typedef struct context_t
{
    const language_t *lang;
    io_t *io;

    void *ast;
} context_t;

typedef struct module_entry_t
{
    lang_handle_t *handle;

    compile_t *compile;
} module_entry_t;

static context_t *context_new(source_t src)
{
    context_t *ctx = ctu_malloc(sizeof(context_t));
    ctx->lang = src.lang;
    ctx->io = src.io;
    return ctx;
}

static module_entry_t *module_new(lang_handle_t *handle, compile_t *compile)
{
    module_entry_t *entry = ctu_malloc(sizeof(module_entry_t));
    entry->handle = handle;
    entry->compile = compile;
    return entry;
}

// public api

lifetime_t *mediator_get_lifetime(mediator_t *self, reports_t *reports)
{
    CTASSERT(self != NULL);

    lifetime_t *lifetime = ctu_malloc(sizeof(lifetime_t));
    lifetime->parent = self;
    lifetime->languages = set_new(4);
    lifetime->handles = map_new(4);
    lifetime->files = vector_new(4);
    lifetime->modules = map_new(8);
    lifetime->reports = reports;
    return lifetime;
}

compile_t *lifetime_add_module(lifetime_t *self, lang_handle_t *handle, const char *name, compile_t *data)
{
    CTASSERT(self != NULL);
    CTASSERT(name != NULL);
    CTASSERT(data != NULL);

    module_entry_t *mod = module_new(handle, data);

    map_set(self->modules, name, mod);
    logverbose("adding module `%s` (0x%p)", name, data);
    return data;
}

compile_t *lifetime_get_module(const lifetime_t *self, const char *name)
{
    CTASSERT(self != NULL);
    CTASSERT(name != NULL);

    logverbose("getting module `%s`", name);

    module_entry_t *mod = map_get(self->modules, name);
    if (mod == NULL) { return NULL; }
    
    return mod->compile;
}

reports_t *lifetime_get_reports(const lifetime_t *self)
{
    CTASSERT(self != NULL);
    return self->reports;
}

void lifetime_add_source(lifetime_t *self, source_t source)
{
    CTASSERT(self != NULL);
    CTASSERT(io_error(source.io) == 0);
    CTASSERT(source.lang != NULL);

    set_add_ptr(self->languages, source.lang);

    vector_push(&self->files, context_new(source));
}

void lifetime_init(lifetime_t *self)
{
    set_iter_t iter = set_iter(self->languages);
    while (set_has_next(&iter))
    {
        const language_t *lang = set_next(&iter);
        lang_handle_t *handle = lang_init(self, lang);
        map_set_ptr(self->handles, lang, handle);
    }
}

void lifetime_deinit(lifetime_t *self)
{
    set_iter_t iter = set_iter(self->languages);
    while (set_has_next(&iter))
    {
        const language_t *lang = set_next(&iter);
        lang_handle_t *handle = map_get_ptr(self->handles, lang);
        EXEC(lang, fnDeinit, handle);
    }
}

void lifetime_parse(reports_t *reports, lifetime_t *self)
{
    size_t len = vector_len(self->files);
    for (size_t i = 0; i < len; i++)
    {
        context_t *src = vector_get(self->files, i);
        const language_t *lang = src->lang;
        lang_handle_t *handle = map_get_ptr(self->handles, lang);
        scan_t *scan = scan_io(reports, lang->name, src->io, handle);

        src->ast = lang->fnParse(handle, scan);

        logverbose("%s:fnParse(%s) = 0x%p", lang->id, io_name(src->io), src->ast);
    }
}

void lifetime_forward(reports_t *reports, lifetime_t *self)
{
    size_t len = vector_len(self->files);
    for (size_t i = 0; i < len; i++)
    {
        context_t *src = vector_get(self->files, i);
        const language_t *lang = src->lang;
        lang_handle_t *handle = map_get_ptr(self->handles, lang);

        char *name = str_filename_noext(io_name(src->io));

        lang->fnForward(handle, name, src->ast);

        logverbose("%s:fnForward(%s)", lang->id, name);
    }
}

void lifetime_import(reports_t *reports, lifetime_t *self)
{
    map_iter_t iter = map_iter(self->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        module_entry_t *mod = entry.value;
        lang_handle_t *handle = mod->handle;
        lang_import(handle, mod->compile);
    }
}

void lifetime_compile(reports_t *reports, lifetime_t *self)
{
    map_iter_t iter = map_iter(self->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        module_entry_t *mod = entry.value;
        lang_compile(mod->handle, mod->compile);
    }
}

vector_t *lifetime_modules(lifetime_t *self)
{
    vector_t *result = vector_new(32);
    map_iter_t iter = map_iter(self->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        module_entry_t *mod = entry.value;
        vector_push(&result, compile_get_module(mod->compile));
    }

    return result;
}
