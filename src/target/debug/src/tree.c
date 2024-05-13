#include "base/panic.h"
#include "cthulhu/events/events.h"
#include "cthulhu/tree/query.h"
#include "debug/target.h"

#include "cthulhu/broker/broker.h"
#include "fs/fs.h"
#include "os/os.h"
#include "io/io.h"
#include "std/map.h"

typedef struct dbg_emit_t
{
    io_t *io;
    size_t indent;
} dbg_emit_t;

static void dbg_emit(dbg_emit_t *emit, const char *fmt, ...)
{
    for (size_t i = 0; i < emit->indent; ++i)
        io_write(emit->io, "  ", 2);

    va_list args;
    va_start(args, fmt);
    io_vprintf(emit->io, fmt, args);
    va_end(args);

    io_write(emit->io, "\n", 1);
}

static void dbg_emit_tree(dbg_emit_t *emit, const tree_t *tree, const char *name)
{
    dbg_emit(emit, "%s: %s (%p)", name, tree_kind_to_string(tree_get_kind(tree)), tree);
}

static void dbg_indent(dbg_emit_t *emit)
{
    emit->indent += 1;
}

static void dbg_dedent(dbg_emit_t *emit)
{
    CTASSERTF(emit->indent > 0, "cannot dedent past 0");
    emit->indent -= 1;
}

static void emit_tree(dbg_emit_t *dbg, const tree_t *tree);

static void emit_storage(dbg_emit_t *dbg, tree_storage_t storage)
{
    dbg_emit(dbg, "type: %s", tree_to_string(storage.storage));
    dbg_emit(dbg, "length: %zu", storage.length);
    dbg_emit(dbg, "qualifiers: %s", quals_string(storage.quals));
}

static void emit_section(dbg_emit_t *dbg, const char *name, map_t *section)
{
    dbg_emit(dbg, "%s:", name);
    dbg_indent(dbg);

    map_iter_t iter = map_iter(section);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        const tree_t *tree = entry.key;
        emit_tree(dbg, tree);
    }

    dbg_dedent(dbg);
}

static void emit_module(dbg_emit_t *dbg, const tree_t *tree)
{
    dbg_emit_tree(dbg, tree, tree_get_name(tree));
    dbg_indent(dbg);

    emit_section(dbg, "types", tree_module_tag(tree, eSemaTypes));
    emit_section(dbg, "values", tree_module_tag(tree, eSemaValues));
    emit_section(dbg, "functions", tree_module_tag(tree, eSemaProcs));
    emit_section(dbg, "modules", tree_module_tag(tree, eSemaModules));

    dbg_dedent(dbg);
}

static void emit_global(dbg_emit_t *dbg, const tree_t *tree)
{
    dbg_emit_tree(dbg, tree, tree_get_name(tree));
    dbg_indent(dbg);
    dbg_emit(dbg, "type:");
    dbg_indent(dbg);
        emit_tree(dbg, tree_get_type(tree));
    dbg_dedent(dbg);
    dbg_emit(dbg, "storage:");
    dbg_indent(dbg);
        emit_storage(dbg, tree_get_storage(tree));
    dbg_dedent(dbg);

    dbg_emit(dbg, "value:");
    dbg_indent(dbg);
        emit_tree(dbg, tree->initial);
    dbg_dedent(dbg);

    dbg_dedent(dbg);
}

static void emit_tree(dbg_emit_t *dbg, const tree_t *tree)
{
    if (tree == NULL)
    {
        dbg_emit(dbg, "NULL");
        return;
    }

    tree_kind_t kind = tree_get_kind(tree);
    switch (tree_get_kind(tree))
    {
    case eTreeDeclModule:
        emit_module(dbg, tree);
        break;
    case eTreeDeclGlobal:
        emit_global(dbg, tree);
        break;
    default:
        dbg_emit(dbg, "%s unimplemented (%p)", tree_kind_to_string(kind), tree);
        break;
    }
}

void debug_tree(target_runtime_t *runtime, const tree_t *tree, target_emit_t *emit)
{
    io_t *io = fs_open(emit->fs, "debug.tree", eOsAccessWrite | eOsAccessTruncate);
    os_error_t err = eOsSuccess;
    if ((err = io_error(io)) != eOsSuccess)
    {
        evt_os_error(runtime->logger, &kEvent_FailedToCreateOutputFile, err, "failed to create debug.tree");
        return;
    }

    dbg_emit_t dbg = {
        .io = io,
        .indent = 0,
    };

    emit_tree(&dbg, tree);

    io_close(io);
}
