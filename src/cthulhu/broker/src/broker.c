#include "cthulhu/broker/broker.h"

#include "base/log.h"
#include "cthulhu/broker/scan.h"
#include "cthulhu/events/events.h"

#include "arena/arena.h"
#include "base/panic.h"
#include "base/util.h"
#include "cthulhu/tree/tree.h"
#include "interop/compile.h"
#include "notify/notify.h"
#include "scan/node.h"
#include "std/map.h"
#include "std/vector.h"
#include <stdio.h>
#include <string.h>

typedef struct broker_t
{
    const frontend_t *frontend;
    arena_t *arena;
    node_t *builtin;
    logger_t *logger;
    tree_cookie_t cookie;
    tree_t *root;

    // all languages
    // vector_t<language_runtime_t*>
    vector_t *langs;

    // all targets
    // vector_t<target_runtime_t*>
    vector_t *targets;

    // all plugins
    // stored in an array rather than a map because
    // they need to be executed in the order they were added
    // vector_t<plugin_runtime_t*>
    vector_t *plugins;

    // all translation units
    // map_t<unit_id_t*, compile_unit_t*>
    map_t *units;

    // all builtin modules
    map_t *builtins;
} broker_t;

static const size_t kDeclSizes[eSemaTotal] = {
    [eSemaValues] = 1,
    [eSemaTypes] = 1,
    [eSemaProcs] = 1,
    [eSemaModules] = 64,
};

#define OPT_EXEC(fn, ...) do { if (fn != NULL) fn(__VA_ARGS__); } while (0)

/// @brief was the parse successful
static bool parse_ok(parse_result_t result, scan_t *scan, logger_t *logger)
{
    switch (result.result)
    {
    case eParseOk: return true;
    case eParseReject: return false;

    case eParseInitError:
        msg_notify(logger, &kEvent_ParseInitFailed, node_new(scan, kNowhere), "failed to initialize parser");
        return false;

    case eParseScanError:
        msg_notify(logger, &kEvent_ScanFailed, node_new(scan, kNowhere), "failed to scan input");
        return false;

    default: NEVER("unknown parse result %d", result.result);
    }
}

static void resolve_module_tag(tree_t *mod, size_t tag)
{
    map_iter_t iter = map_iter(tree_module_tag(mod, tag));
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        tree_t *tree = entry.value;
        tree_resolve(tree_get_cookie(mod), tree);
    }
}

static void resolve_module(tree_t *mod)
{
    CTASSERT(mod != NULL);

    resolve_module_tag(mod, eSemaValues);
    resolve_module_tag(mod, eSemaTypes);
    resolve_module_tag(mod, eSemaProcs);

    map_iter_t mods = map_iter(tree_module_tag(mod, eSemaModules));
    while (map_has_next(&mods))
    {
        map_entry_t entry = map_next(&mods);
        tree_t *tree = entry.value;
        resolve_module(tree);
    }
}

static compile_unit_t *compile_unit_new(language_runtime_t *lang, arena_t *arena, void *ast, tree_t *tree)
{
    CTASSERT(lang != NULL);
    CTASSERT(arena != NULL);
    CTASSERT(tree != NULL);
    broker_t *broker = lang->broker;

    compile_unit_t *unit = ARENA_MALLOC(sizeof(compile_unit_t), "compilation unit", lang, arena);
    unit->lang = lang;
    unit->ast = ast;
    unit->tree = tree;
    tree_module_set(broker->root, eSemaModules, tree_get_name(tree), tree);

    ARENA_REPARENT(tree, unit, arena);

    return unit;
}

///
/// broker api
///

USE_DECL
broker_t *broker_new(const frontend_t *frontend, arena_t *arena)
{
    CTASSERT(frontend != NULL);
    CTASSERT(arena != NULL);

    module_info_t info = frontend->info;

    broker_t *broker = ARENA_MALLOC(sizeof(broker_t), "broker", NULL, arena);
    broker->frontend = frontend;
    broker->arena = arena;
    broker->builtin = node_builtin(info.name, arena);
    broker->logger = logger_new(arena);

    ARENA_REPARENT(broker->builtin, broker, arena);
    ARENA_REPARENT(broker->logger, broker, arena);

    tree_cookie_t cookie = {
        .reports = broker->logger,
        .stack = vector_new(16, arena)
    };

    ARENA_REPARENT(cookie.stack, broker, arena);

    broker->cookie = cookie;
    broker->root = tree_module_root(broker->logger, &broker->cookie, broker->builtin, info.name, eSemaTotal, kDeclSizes, arena);

    broker->langs = vector_new(8, arena);
    broker->targets = vector_new(8, arena);
    broker->plugins = vector_new(8, arena);

    broker->units = map_new(64, kTypeInfoText, arena);
    broker->builtins = map_new(64, kTypeInfoText, arena);

    ARENA_REPARENT(broker->root, broker, arena);
    ARENA_REPARENT(broker->langs, broker, arena);
    ARENA_REPARENT(broker->targets, broker, arena);
    ARENA_REPARENT(broker->plugins, broker, arena);
    ARENA_REPARENT(broker->units, broker, arena);
    ARENA_REPARENT(broker->builtins, broker, arena);

    return broker;
}

USE_DECL
language_runtime_t *broker_add_language(broker_t *broker, const language_t *lang)
{
    CTASSERT(broker != NULL);
    CTASSERT(lang != NULL);

    arena_t *arena = broker->arena;
    module_info_t info = lang->info;
    CTASSERT(info.id != NULL);
    CTASSERT(info.name != NULL);
    CTASSERT(info.version.license != NULL);

    language_info_t builtin = lang->builtin;
    CTASSERTF(builtin.name.text != NULL, "language '%s' did not specify a builtin name", info.name);
    CTASSERTF(builtin.length >= eSemaTotal, "language '%s' did not specify builtin decls", info.name);
    CTASSERTF(builtin.decls != NULL, "language '%s' did not specify builtin decls", info.name);

    language_runtime_t *runtime = ARENA_MALLOC(sizeof(language_runtime_t), info.name, broker, arena);
    runtime->info = lang;
    runtime->broker = broker;

    // TODO: seperate arena per language
    runtime->arena = arena;
    runtime->logger = broker->logger;

    if (lang->ast_size != 0)
    {
        runtime->ast_bitmap = bitmap_new(0x1000, lang->ast_size, arena);
        ARENA_IDENTIFY(runtime->ast_bitmap, "ast bitmap arena", runtime, arena);

        arena_t ast_arena = {
            .name = "ast",
            .parent = arena,
        };

        bitmap_arena_init(&ast_arena, runtime->ast_bitmap);
        runtime->ast_arena = ast_arena;
    }
    else
    {
        runtime->ast_bitmap = NULL;
        memset(&runtime->ast_arena, 0, sizeof(arena_t));
    }

    node_t *node = node_builtin(info.id, arena);
    ARENA_REPARENT(node, runtime, arena);

    // all builtins for the language go into this module
    tree_t *tree = tree_module(broker->root, node, info.id, builtin.length, builtin.decls);
    ARENA_REPARENT(tree, runtime, arena);

    runtime->root = tree;
    vector_push(&broker->langs, runtime);

    compile_unit_t *unit = compile_unit_new(runtime, arena, NULL, tree);

    map_set(broker->builtins, &builtin.name, unit);

    tree_module_set(broker->root, eSemaModules, info.id, tree);

    return runtime;
}

USE_DECL
plugin_runtime_t *broker_add_plugin(broker_t *broker, const plugin_t *plugin)
{
    CTASSERT(broker != NULL);
    CTASSERT(plugin != NULL);

    arena_t *arena = broker->arena;
    module_info_t info = plugin->info;

    CTASSERT(info.id != NULL);
    CTASSERT(info.name != NULL);
    CTASSERT(info.version.license != NULL);

    plugin_runtime_t *runtime = ARENA_MALLOC(sizeof(plugin_runtime_t), info.name, broker, arena);
    runtime->info = plugin;

    vector_push(&broker->plugins, runtime);

    return runtime;
}

USE_DECL
target_runtime_t *broker_add_target(broker_t *broker, const target_t *target)
{
    CTASSERT(broker != NULL);
    CTASSERT(target != NULL);

    arena_t *arena = broker->arena;
    module_info_t info = target->info;

    CTASSERT(info.id != NULL);
    CTASSERT(info.name != NULL);
    CTASSERT(info.version.license != NULL);

    target_runtime_t *runtime = ARENA_MALLOC(sizeof(target_runtime_t), info.name, broker, arena);
    runtime->info = target;

    vector_push(&broker->targets, runtime);

    return runtime;
}

USE_DECL
void broker_init(broker_t *broker)
{
    CTASSERT(broker != NULL);

    // init plugins
    size_t len = vector_len(broker->plugins);
    for (size_t i = 0; i < len; i++)
    {
        plugin_runtime_t *plugin = vector_get(broker->plugins, i);
        OPT_EXEC(plugin->info->fn_create, plugin);
    }

    // init targets
    len = vector_len(broker->targets);
    for (size_t i = 0; i < len; i++)
    {
        target_runtime_t *target = vector_get(broker->targets, i);
        OPT_EXEC(target->info->fn_create, target);
    }

    // init languages
    len = vector_len(broker->langs);
    for (size_t i = 0; i < len; i++)
    {
        language_runtime_t *lang = vector_get(broker->langs, i);
        OPT_EXEC(lang->info->fn_create, lang, lang->root);
    }
}

USE_DECL
void broker_deinit(broker_t *broker)
{
    CTASSERT(broker != NULL);

    // deinit languages
    size_t len = vector_len(broker->langs);
    for (size_t i = 0; i < len; i++)
    {
        language_runtime_t *lang = vector_get(broker->langs, i);
        OPT_EXEC(lang->info->fn_destroy, lang);
    }

    // deinit targets
    len = vector_len(broker->targets);
    for (size_t i = 0; i < len; i++)
    {
        target_runtime_t *target = vector_get(broker->targets, i);
        OPT_EXEC(target->info->fn_destroy, target);
    }

    // deinit plugins
    len = vector_len(broker->plugins);
    for (size_t i = 0; i < len; i++)
    {
        plugin_runtime_t *plugin = vector_get(broker->plugins, i);
        OPT_EXEC(plugin->info->fn_destroy, plugin);
    }
}

USE_DECL
logger_t *broker_get_logger(broker_t *broker)
{
    CTASSERT(broker != NULL);

    return broker->logger;
}

USE_DECL
const node_t *broker_get_node(broker_t *broker)
{
    CTASSERT(broker != NULL);

    return broker->builtin;
}

USE_DECL
arena_t *broker_get_arena(broker_t *broker)
{
    CTASSERT(broker != NULL);

    return broker->arena;
}

static void collect_units(vector_t **vec, map_t *map)
{
    map_iter_t iter = map_iter(map);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        compile_unit_t *unit = entry.value;
        CTASSERT(unit != NULL);

        vector_push(vec, unit->tree);
    }
}

USE_DECL
vector_t *broker_get_modules(broker_t *broker)
{
    CTASSERT(broker != NULL);

    vector_t *modules = vector_new(64, broker->arena);
    collect_units(&modules, broker->builtins);
    collect_units(&modules, broker->units);

    ARENA_IDENTIFY(modules, "modules", broker, broker->arena);

    return modules;
}

USE_DECL
void broker_parse(language_runtime_t *runtime, io_t *io)
{
    CTASSERT(runtime != NULL);
    CTASSERT(io != NULL);

    broker_t *broker = runtime->broker;

    const language_t *lang = runtime->info;
    const module_info_t *info = &lang->info;

    scan_context_t *ctx = ARENA_MALLOC(sizeof(scan_context_t) + lang->context_size, "scan context", runtime, broker->arena);
    ctx->logger = broker->logger;
    ctx->arena = broker->arena;
    ctx->string_arena = broker->arena;
    ctx->ast_arena = broker->arena;

    // TODO: allow languages that dont use scanner callbacks
    CTASSERTF(lang->scanner != NULL, "language '%s' did not specify a scanner", info->name);

    scan_t *scan = scan_io(info->name, io, broker->arena);
    ARENA_REPARENT(scan, runtime, broker->arena);

    if (lang->fn_preparse != NULL)
    {
        lang->fn_preparse(runtime, ctx->user);
        scan_set_context(scan, ctx->user); // for backwards compatibility
    }
    else
    {
        scan_set_context(scan, ctx);
    }

    parse_result_t result = scan_buffer(scan, lang->scanner);
    if (!parse_ok(result, scan, broker->logger))
    {
        return;
    }

    CTASSERTF(lang->fn_postparse != NULL, "language '%s' did not specify a postparse function", info->name);
    lang->fn_postparse(runtime, scan, result.tree);
}

USE_DECL
void broker_run_pass(broker_t *broker, broker_pass_t pass)
{
    CTASSERT(broker != NULL);

    map_iter_t iter = map_iter(broker->units);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        compile_unit_t *unit = entry.value;
        CTASSERT(unit != NULL);

        if (unit->ast == NULL)
            continue;

        language_runtime_t *lang = unit->lang;
        CTASSERTF(lang != NULL, "unit '%s' has no associated language", tree_get_name(unit->tree));

        language_pass_t fn = lang->info->fn_passes[pass];
        if (fn == NULL)
        {
            const language_t *it = lang->info;
            const module_info_t *info = &it->info;
            ctu_log("language '%s' does not implement pass '%s'", info->name, broker_pass_name(pass));
            continue;
        }

        OPT_EXEC(fn, lang, unit);
    }
}

USE_DECL
void broker_resolve(broker_t *broker)
{
    CTASSERT(broker != NULL);

    resolve_module(broker->root);
}

///
/// translation unit api
///

USE_DECL
void lang_add_unit(language_runtime_t *runtime, unit_id_t id, const node_t *node, void *ast, const size_t *decls, size_t length)
{
    CTASSERT(runtime != NULL);
    CTASSERT(id.text != NULL);
    CTASSERT(id.length > 0);
    CTASSERT(ast != NULL);
    CTASSERT(decls != NULL);
    CTASSERT(length >= eSemaTotal);

    compile_unit_t *old = map_get(runtime->broker->units, &id);
    if (old != NULL)
    {
        msg_notify(runtime->logger, &kEvent_ModuleConflict, tree_get_node(old->tree), "module '%s' already exists", id.text);
        return;
    }
    arena_t *arena = runtime->arena;

    char *copy = arena_memdup(id.text, id.length, arena);
    for (size_t i = 0; i < id.length; i++)
        if (copy[i] == '\0')
            copy[i] = '/';
    copy[id.length] = '\0';

    tree_t *tree = tree_module(runtime->root, node, copy, length, decls);
    ARENA_REPARENT(copy, tree, arena);

    compile_unit_t *unit = compile_unit_new(runtime, arena, ast, tree);

    text_view_t *key = arena_memdup(&id, sizeof(unit_id_t), arena);
    map_set(runtime->broker->units, key, unit);
}

compile_unit_t *lang_get_unit(language_runtime_t *runtime, unit_id_t id)
{
    CTASSERT(runtime != NULL);
    CTASSERT(id.text != NULL);
    CTASSERT(id.length > 0);

    broker_t *broker = runtime->broker;
    compile_unit_t *builtin = map_get(broker->builtins, &id);
    if (builtin != NULL)
    {
        return builtin;
    }

    return map_get(broker->units, &id);
}

USE_DECL
void *unit_get_ast(compile_unit_t *unit)
{
    CTASSERT(unit != NULL);

    return unit->ast;
}

USE_DECL
void unit_update(compile_unit_t *unit, void *ast, tree_t *tree)
{
    CTASSERT(unit != NULL);
    CTASSERT(ast != NULL);
    CTASSERT(tree != NULL);

    CTASSERTF(unit->ast != NULL, "attempting to modify builtin module '%s'", tree_get_name(unit->tree));

    unit->ast = ast;
    unit->tree = tree;
}

USE_DECL
text_view_t build_unit_id(const vector_t *parts, arena_t *arena)
{
    CTASSERT(parts != NULL);
    CTASSERT(arena != NULL);

    size_t len = vector_len(parts);
    size_t chars = 0;
    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        chars += ctu_strlen(part);
    }

    size_t size = chars + len - 1;
    char *buf = ARENA_MALLOC(size + 1, "unit_id", NULL, arena);

    size_t offset = 0;
    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        size_t part_len = ctu_strlen(part);
        ctu_memcpy(buf + offset, part, part_len);
        offset += part_len;
        if (i + 1 < len)
        {
            buf[offset++] = '\0';
        }
    }

    buf[size] = '\0';
    return text_view_make(buf, size);
}

static const char *const kPassNames[ePassCount] = {
#define BROKER_PASS(ID, STR) [ID] = (STR),
#include "cthulhu/broker/broker.def"
};

const char *broker_pass_name(broker_pass_t pass)
{
    CTASSERTF(pass < ePassCount, "invalid pass %d", pass);
    return kPassNames[pass];
}
