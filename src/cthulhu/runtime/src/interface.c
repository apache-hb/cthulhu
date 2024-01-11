#include "base/log.h"
#include "common.h"

#include "cthulhu/events/events.h"

#include "cthulhu/runtime/interface.h"
#include "cthulhu/runtime/driver.h"

#include "base/panic.h"

#include "notify/notify.h"
#include "scan/scan.h"
#include "scan/node.h"

#include "interop/compile.h"

#include "std/typed/vector.h"
#include "std/vector.h"
#include "std/str.h"

#include "os/os.h"

#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

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

    driver_t *self = ARENA_MALLOC(lifetime->arena, sizeof(driver_t), lang->id, lifetime);

    self->parent = lifetime;
    self->lang = lang;

    return self;
}

static bool ctx_requires_compile(const context_t *ctx)
{
    return ctx->ast != NULL;
}

lifetime_t *handle_get_lifetime(driver_t *handle)
{
    CTASSERT(handle != NULL);

    return handle->parent;
}

mediator_t *mediator_new(arena_t *arena)
{
    mediator_t *self = ARENA_MALLOC(arena, sizeof(mediator_t), "mediator", NULL);

    self->arena = arena;

    return self;
}

lifetime_t *lifetime_new(mediator_t *mediator, arena_t *arena)
{
    CTASSERT(mediator != NULL);
    CTASSERT(arena != NULL);

    lifetime_t *self = ARENA_MALLOC(arena, sizeof(lifetime_t), "lifetime", mediator);

    logger_t *logger = logger_new(arena);

    self->parent = mediator;

    self->logger = logger;
    self->arena = arena;

    self->extensions = map_optimal(16, kTypeInfoString, arena);
    self->modules = map_optimal(64, kTypeInfoString, arena);

    cookie_t cookie = {
        .reports = logger,
        .stack = vector_new_arena(16, arena),
    };

    self->cookie = cookie;

    return self;
}

USE_DECL
const char *stage_to_string(compile_stage_t stage)
{
#define STAGE(ID, STR) case ID: return STR;
    switch (stage)
    {
#include "cthulhu/runtime/mediator.inc"
    default: return "unknown";
    }
}

void lifetime_add_language(lifetime_t *lifetime, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);

    CTASSERTF(lang->fn_create != NULL, "language `%s` has no create function", lang->id);

    CTASSERT(lang->exts != NULL);

    for (size_t i = 0; lang->exts[i]; i++)
    {
        const language_t *old = add_language_extension(lifetime, lang->exts[i], lang);
        if (old == NULL)
        {
            continue;
        }

        msg_notify(lifetime->logger, &kEvent_ExtensionConflict, node_builtin(), "language `%s` registered under extension `%s` clashes with previously registered language `%s`", lang->id, lang->exts[i], old->id); // TODO: handle this
    }

    driver_t *handle = handle_new(lifetime, lang);
    EXEC(lang, fn_create, handle);
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

static bool parse_failed(logger_t *reports, const char *path, parse_result_t result)
{
    typevec_t *events = logger_get_events(reports);

    switch (result.result)
    {
    case eParseInitFailed:
        msg_notify(reports, &kEvent_ParseInitFailed, node_builtin(), "failed to init parser %s: %d", path, result.error);
        return true;
    case eParseScanFailed:
        msg_notify(reports, &kEvent_ScanFailed, node_builtin(), "failed to scan %s: %d", path, result.error);
        return true;

    // the driver will reject the file with errors, no need to clutter the output
    case eParseFailed:
        return true;

    default:
        // TODO: this needs another rewrite of the mediator module
        if (typevec_len(events) > 0)
        {
            return true;
        }

        return false;
    }
}

void lifetime_parse(lifetime_t *lifetime, const language_t *lang, io_t *io)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);
    CTASSERT(io != NULL);

    scan_t *scan = scan_io(lang->id, io, lifetime->arena);
    scan_set_context(scan, lifetime->logger);

    driver_t *handle = handle_new(lifetime, lang);

    if (lang->parse_callbacks != NULL)
    {
        if (lang->fn_preparse != NULL)
        {
            void *ctx = lang->fn_preparse(handle, scan);
            scan_set_context(scan, ctx);
        }

        parse_result_t result = scan_buffer(scan, lang->parse_callbacks);
        const char *path = scan_path(scan);
        if (parse_failed(lifetime->logger, path, result))
        {
            ctu_log("parse failed for %s", path);
            return;
        }

        CTASSERTF(lang->fn_postparse != NULL, "language `%s` has no postpass function", lang->id);
        if (result.result == eParseOk)
        {
            lang->fn_postparse(handle, scan, result.tree);
        }
    }
    else
    {
        CTASSERTF(lang->fn_parse != NULL, "language `%s` has no parse function", lang->id);
        lang->fn_parse(handle, scan);
    }
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
        driver_pass_t fn_pass = lang->fn_compile_passes[stage];

        if (fn_pass == NULL || !ctx_requires_compile(ctx))
        {
            continue;
        }

        fn_pass(ctx);
    }
}

map_t *lifetime_get_modules(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    arena_t *arena = lifetime->arena;

    map_t *mods = map_optimal(64, kTypeInfoString, arena);
    ARENA_IDENTIFY(arena, mods, "modules", lifetime);

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
