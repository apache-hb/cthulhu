#include "cfamily/target.h"

#include "core/macros.h"
#include "cthulhu/broker/broker.h"

#include "fs/fs.h"
#include "io/io.h"
#include "os/os.h"
#include "base/panic.h"
#include "cthulhu/events/events.h"

typedef struct cfamily_emit_t
{
    vector_t *names;
} cfamily_emit_t;

static void cfamily_pair(target_runtime_t *runtime, const tree_t *tree, fs_t *fs)
{
    CT_UNUSED(tree);

    os_error_t err = eOsSuccess;
    io_t *src = fs_open(fs, "tree.c", eOsAccessWrite | eOsAccessTruncate);
    io_t *hdr = fs_open(fs, "tree.h", eOsAccessWrite | eOsAccessTruncate);

    if ((err = io_error(src)) != eOsSuccess)
    {
        evt_os_error(runtime->logger, &kEvent_FailedToCreateOutputFile, err, "failed to create tree.c");
        return;
    }

    if ((err = io_error(hdr)) != eOsSuccess)
    {
        evt_os_error(runtime->logger, &kEvent_FailedToCreateOutputFile, err, "failed to create tree.h");
        return;
    }

    io_printf(src, "/* This file was generated, do not edit */\n\n");
    io_printf(src, "#include \"tree.h\"\n\n");

    io_printf(hdr, "/* This file was generated, do not edit */\n\n");
    io_printf(hdr, "#pragma once\n\n");

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
