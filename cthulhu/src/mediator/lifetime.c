#include "cthulhu/mediator/mediator.h"

#include "cthulhu/mediator/language.h"

#include "scan/compile.h"

#include "std/vector.h"
#include "std/set.h"
#include "std/map.h"

#include "io/io.h"

#include "base/util.h"
#include "base/panic.h"
#include "base/memory.h"

#include "common.h"

typedef struct lifetime_t
{
    mediator_t *parent;

    set_t *languages;
    map_t *handles; // map_t<const language_t *, lang_handle_t *>

    vector_t *files; /// vector_t<source_t>
} lifetime_t;

typedef struct context_t
{
    const language_t *lang;
    io_t *io;

    void *ast;
} context_t;

static context_t *context_new(source_t src)
{
    context_t *ctx = ctu_malloc(sizeof(context_t));
    ctx->lang = src.lang;
    ctx->io = src.io;
    return ctx;
}

// public api

lifetime_t *mediator_get_lifetime(mediator_t *self)
{
    CTASSERT(self != NULL);

    lifetime_t *lifetime = ctu_malloc(sizeof(lifetime_t));
    lifetime->parent = self;
    lifetime->languages = set_new(4);
    lifetime->handles = map_new(4);
    lifetime->files = vector_new(4);
    return lifetime;
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
        lang_handle_t *handle = init_language(self, lang);
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
    }
}
