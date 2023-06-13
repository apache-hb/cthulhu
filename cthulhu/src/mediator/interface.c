#include "common.h"

#include "cthulhu/mediator/interface.h"

#include "base/memory.h"
#include "base/panic.h"

#include "scan/scan.h"

#include "std/vector.h"

#include "report/report.h"

#include "platform/error.h"
#include "stacktrace/stacktrace.h"
#include "cthulhu/hlir/init.h"

static void runtime_init(void)
{
    GLOBAL_INIT();

    stacktrace_init();
    platform_init();
    init_gmp(&globalAlloc);
    init_hlir();
}

static const language_t *add_language_extension(lifetime_t *lifetime, const char *ext, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(ext != NULL);
    CTASSERT(lang != NULL);

    logverbose("mapping language `%s` to extension `.%s`", lang->id, ext);

    const language_t *old = map_get(lifetime->extensions, ext);
    if (old != NULL)
    {
        return old;
    }

    map_set(lifetime->extensions, ext, (void*)lang);
    return NULL;
}

static handle_t *handle_new(lifetime_t *lifetime, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);

    handle_t *self = ctu_malloc(sizeof(handle_t));

    self->parent = lifetime;
    self->lang = lang;

    return self;
}

bool context_requires_compiling(const context_t *ctx)
{
    return ctx->ast != NULL;
}

lifetime_t *handle_get_lifetime(handle_t *handle)
{
    CTASSERT(handle != NULL);

    return handle->parent;
}

mediator_t *mediator_new(const char *id, version_info_t version)
{
    CTASSERT(id != NULL);

    runtime_init();

    mediator_t *self = ctu_malloc(sizeof(mediator_t));

    self->id = id;
    self->version = version;

    return self;
}

lifetime_t *lifetime_new(mediator_t *mediator)
{
    CTASSERT(mediator != NULL);

    lifetime_t *self = ctu_malloc(sizeof(lifetime_t));

    self->parent = mediator;

    self->reports = begin_reports();
    self->extensions = map_new(16);
    self->modules = map_new(64);

    return self;
}

const char *stage_to_string(compile_stage_t stage)
{
#define STAGE(ID, STR) case ID: return STR;
    switch (stage)
    {
#include "cthulhu/mediator/mediator-def.inc"
    default: return "unknown";
    }
}

void lifetime_add_language(lifetime_t *lifetime, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);

    CTASSERT(lang->fnCreate != NULL);

    for (size_t i = 0; lang->exts[i] != NULL; i++)
    {
        const language_t *old = add_language_extension(lifetime, lang->exts[i], lang);
        if (old == NULL)
        {
            continue;
        }

        report(lifetime->reports, eInternal, node_invalid(), "language `%s` registered under extension `%s` clashes with previously registered language `%s`", lang->id, lang->exts[i], old->id); // TODO: handle this
    }
    
    handle_t *handle = handle_new(lifetime, lang);
    EXEC(lang, fnCreate, handle);
}

const language_t *lifetime_get_language(lifetime_t *lifetime, const char *ext)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(ext != NULL);

    return map_get(lifetime->extensions, ext);
}

void lifetime_parse(lifetime_t *lifetime, const language_t *lang, io_t *io)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);
    CTASSERT(io != NULL);

    CTASSERT(lang->fnParse != NULL);

    scan_t *scan = scan_io(lifetime->reports, lang->id, io, lifetime);
    handle_t *handle = handle_new(lifetime, lang);

    logverbose("%s:fnParse(%s)", lang->id, scan_path(scan));
    lang->fnParse(handle, scan);
}

void lifetime_run_stage(lifetime_t *lifetime, compile_stage_t stage)
{
    CTASSERT(lifetime != NULL);

    const char *name = stage_to_string(stage);    
    logverbose("=== %s ===", name);

    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;
        const char *id = entry.key;

        CTASSERT(ctx != NULL);

        const language_t *lang = ctx->lang;
        compile_pass_t fnPass = lang->fnCompilePass[stage];

        if (!context_requires_compiling(ctx) || fnPass == NULL)
        {
            logverbose("skipped %s:fnCompilePass(%s)", lang->id, id);
            continue;
        }

        logverbose("execute %s:fnCompilePass(%s)", lang->id, id);
        fnPass(ctx);
    }
}

vector_t *lifetime_get_modules(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    vector_t *mods = vector_new(64);

    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;
        const char *name = entry.key;

        CTASSERTF(ctx != NULL, "module `%s` is NULL", name);
        CTASSERTF(ctx->root != NULL, "module `%s` has NULL root", name);

        vector_push(&mods, ctx->root);
    }

    logverbose("acquired modules %zu", vector_len(mods));

    return mods;
}
