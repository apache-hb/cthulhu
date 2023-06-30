#include "common.h"

#include "cthulhu/mediator/interface.h"

#include "base/memory.h"
#include "base/panic.h"

#include "scan/scan.h"

#include "std/vector.h"

#include "report/report.h"

#include "stacktrace/stacktrace.h"
#include "cthulhu/hlir/h2.h"
#include "cthulhu/hlir/check.h"

static void runtime_init(void)
{
    GLOBAL_INIT();

    stacktrace_init();
    init_gmp(&globalAlloc);
}

static const language_t *add_language_extension(lifetime_t *lifetime, const char *ext, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(ext != NULL);
    CTASSERT(lang != NULL);

    const language_t *old = map_get(lifetime->extensions, ext);
    if (old != NULL)
    {
        return old;
    }

    map_set(lifetime->extensions, ext, (void*)lang);
    return NULL;
}

static driver_t *handle_new(lifetime_t *lifetime, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);

    driver_t *self = ctu_malloc(sizeof(driver_t));

    self->parent = lifetime;
    self->lang = lang;

    return self;
}

bool context_requires_compiling(const context_t *ctx)
{
    return ctx->ast != NULL;
}

lifetime_t *handle_get_lifetime(driver_t *handle)
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
#include "cthulhu/mediator/mediator.inc"
    default: return "unknown";
    }
}

void lifetime_config_language(lifetime_t *lifetime, ap_t *ap, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(ap != NULL);
    CTASSERT(lang != NULL);

    if (lang->fnConfig == NULL) { return; }

    EXEC(lang, fnConfig, lifetime, ap);
}

void lifetime_add_language(lifetime_t *lifetime, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);

    CTASSERTF(lang->fnCreate != NULL, "language `%s` has no create function", lang->id);

    for (size_t i = 0; lang->exts[i] != NULL; i++)
    {
        const language_t *old = add_language_extension(lifetime, lang->exts[i], lang);
        if (old == NULL)
        {
            continue;
        }

        report(lifetime->reports, eInternal, node_invalid(), "language `%s` registered under extension `%s` clashes with previously registered language `%s`", lang->id, lang->exts[i], old->id); // TODO: handle this
    }
    
    driver_t *handle = handle_new(lifetime, lang);
    EXEC(lang, fnCreate, handle);
}

const language_t *lifetime_add_extension(lifetime_t *lifetime, const char *ext, const language_t *lang)
{
    return add_language_extension(lifetime, ext, lang);
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

    scan_t *scan = scan_io(lifetime->reports, lang->id, io);
    driver_t *handle = handle_new(lifetime, lang);

    lang->fnParse(handle, scan);
}

void lifetime_run_stage(lifetime_t *lifetime, compile_stage_t stage)
{
    CTASSERT(lifetime != NULL);

    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;

        CTASSERT(ctx != NULL);

        const language_t *lang = ctx->lang;
        compile_pass_t fnPass = lang->fnCompilePass[stage];

        if (!context_requires_compiling(ctx) || fnPass == NULL)
        {
            continue;
        }

        fnPass(ctx);
    }
}

map_t *lifetime_get_modules(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    map_t *mods = map_optimal(64);

    size_t items = 0;
    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;
        const char *name = entry.key;

        CTASSERTF(ctx != NULL, "module `%s` is NULL", name);
        CTASSERTF(ctx->root != NULL, "module `%s` has NULL root", name);

        logverbose("acquiring module `%s`", name);

        map_set(mods, name, ctx->root);
        items += 1;
    }

    logverbose("acquired %zu modules", items);

    return mods;
}

void lifetime_check(lifetime_t *lifetime)
{
    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;
        const char *name = entry.key;

        CTASSERTF(ctx != NULL, "module `%s` is NULL", name);
        CTASSERTF(ctx->root != NULL, "module `%s` has NULL root", name);

        h2_check(lifetime_get_reports(lifetime), ctx->root);
    }
}
