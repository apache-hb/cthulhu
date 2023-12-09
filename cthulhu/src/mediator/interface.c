#include "common.h"

#include "cthulhu/mediator/interface.h"
#include "cthulhu/mediator/driver.h"

#include "memory/memory.h"
#include "base/panic.h"

#include "scan/scan.h"
#include "scan/node.h"

#include "std/vector.h"
#include "std/str.h"

#include "report/report.h"

#include "os/os.h"

#include "stacktrace/stacktrace.h"

#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

static cookie_t *cookie_new(lifetime_t *lifetime, reports_t *reports)
{
    cookie_t *self = ARENA_MALLOC(lifetime->alloc, sizeof(cookie_t), "cookie", lifetime);
    self->reports = reports;
    self->stack = vector_new(16);
    return self;
}

static void runtime_init(void)
{
    GLOBAL_INIT();

    os_init();
    stacktrace_init();

    init_global_alloc(ctu_default_alloc());
    init_gmp_alloc(ctu_default_alloc());
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

    driver_t *self = ARENA_MALLOC(lifetime->alloc, sizeof(driver_t), lang->id, lifetime);

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

mediator_t *mediator_new_noinit(const char *id, version_info_t version)
{
    CTASSERT(id != NULL);

    mediator_t *self = MEM_ALLOC(sizeof(mediator_t), id, NULL);

    self->id = id;
    self->version = version;

    return self;
}

mediator_t *mediator_new(const char *id, version_info_t version)
{
    runtime_init();

    return mediator_new_noinit(id, version);
}

lifetime_t *lifetime_new(mediator_t *mediator, arena_t *alloc)
{
    CTASSERT(mediator != NULL);
    CTASSERT(alloc != NULL);

    lifetime_t *self = ARENA_MALLOC(alloc, sizeof(lifetime_t), "lifetime", mediator);

    self->parent = mediator;

    self->reports = begin_reports();
    self->alloc = alloc;

    self->extensions = map_optimal(16);
    self->modules = map_optimal(64);

    self->cookie = cookie_new(self, self->reports);

    return self;
}

USE_DECL
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

    scan_t *scan = scan_io(lifetime->reports, lang->id, io, lifetime->alloc);
    driver_t *handle = handle_new(lifetime, lang);

    lang->fnParse(handle, scan);
}

static void resolve_tag(tree_t *mod, size_t tag)
{
    map_iter_t iter = map_iter(tree_module_tag(mod, tag));
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        tree_t *decl = entry.value;
        tree_resolve(tree_get_cookie(mod), decl);
    }
}

static void resolve_decls(tree_t *mod)
{
    CTASSERT(mod != NULL);

    resolve_tag(mod, eSemaValues);
    resolve_tag(mod, eSemaTypes);
    resolve_tag(mod, eSemaProcs);

    map_iter_t iter = map_iter(tree_module_tag(mod, eSemaModules));
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        tree_t *child = entry.value;
        resolve_decls(child);
    }
}

void lifetime_resolve(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;

        CTASSERT(ctx != NULL);

        resolve_decls(context_get_module(ctx));
    }
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
        driver_pass_t fnPass = lang->fnCompilePass[stage];

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
    MEM_IDENTIFY(mods, "modules", lifetime);

    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;
        const char *name = entry.key;

        CTASSERTF(ctx != NULL, "module `%s` is NULL", name);
        CTASSERTF(ctx->root != NULL, "module `%s` has NULL root", name);

        map_set(mods, name, ctx->root);
    }

    return mods;
}
