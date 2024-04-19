#include "cfamily/target.h"

#include "core/macros.h"
#include "cthulhu/broker/broker.h"

#include "cthulhu/tree/query.h"
#include "fs/fs.h"
#include "io/io.h"
#include "os/os.h"
#include "base/panic.h"
#include "cthulhu/events/events.h"

#include "std/map.h"
#include "std/vector.h"

typedef struct cfamily_emit_t
{
    vector_t *stack;

    map_t *names; // map of tree_t* -> mangled name

    io_t *src;
    io_t *hdr;
} cfamily_emit_t;

static void push_namespace(cfamily_emit_t *emit, const tree_t *tree)
{
    vector_push(&emit->stack, (void*)tree_get_name(tree));
}

static void pop_namespace(cfamily_emit_t *emit)
{
    vector_drop(emit->stack);
}

static void forward_decl(cfamily_emit_t *emit, const tree_t *tree)
{
    switch (tree_get_kind(tree))
    {
    case eTreeDeclModule:
    {
        push_namespace(emit, tree);

        // map_t *vars = tree_module_tag(tree, eSemaTypes);
        // map_iter_t iter = map_iter(vars);
        // while (map_has_next(&iter))
        // {
        //     (void)map_next(&iter);
        // }

        pop_namespace(emit);
        break;
    }

    default:
        CT_NEVER("unsupported tree kind %s", tree_kind_to_string(tree_get_kind(tree)));
    }
}

static void cfamily_pair(target_runtime_t *runtime, const tree_t *tree, fs_t *dst)
{
    CT_UNUSED(tree);

    os_error_t err = eOsSuccess;
    io_t *src = fs_open(dst, "tree.c", eOsAccessWrite | eOsAccessTruncate);
    io_t *hdr = fs_open(dst, "tree.h", eOsAccessWrite | eOsAccessTruncate);

    if ((err = io_error(src)) != eOsSuccess)
    {
        evt_os_error(runtime->logger, &kEvent_FailedToCreateOutputFile, err, "failed to create tree.c");
        goto cleanup;
    }

    if ((err = io_error(hdr)) != eOsSuccess)
    {
        evt_os_error(runtime->logger, &kEvent_FailedToCreateOutputFile, err, "failed to create tree.h");
        goto cleanup;
    }

    io_printf(src, "/* This file was generated, do not edit */\n\n");
    io_printf(src, "#include \"tree.h\"\n\n");

    io_printf(hdr, "/* This file was generated, do not edit */\n\n");
    io_printf(hdr, "#pragma once\n\n");

    cfamily_emit_t emit = {
        .stack = vector_new(4, runtime->arena),
        .names = map_optimal(1024, kTypeInfoPtr, runtime->arena), // TODO: calculate optimal size
        .src = src,
        .hdr = hdr,
    };

    forward_decl(&emit, tree);

cleanup:
    io_close(src);
    io_close(hdr);
}

void cfamily_tree(target_runtime_t *runtime, const tree_t *tree, target_emit_t *emit)
{
    switch (emit->layout)
    {
    case eFileLayoutPair:
        cfamily_pair(runtime, tree, emit->fs);
        break;

    default:
        CT_NEVER("unsupported file layout %s", file_layout_name(emit->layout));
    }
}
