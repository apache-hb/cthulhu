#include "scan/node.h"

#include "base/panic.h"
#include "arena/arena.h"

typedef struct node_t
{
    const scan_t *scan;  ///< the source file
    where_t where; ///< the location of this node in the source file
} node_t;

static const node_t kNodeBuiltin = {
    .scan = &kScanBuiltin,
    .where = { 0, 0, 0, 0 }
};

USE_DECL
const node_t *node_builtin(void)
{
    return &kNodeBuiltin;
}

USE_DECL
bool node_is_builtin(const node_t *node)
{
    return node == node_builtin();
}

USE_DECL
node_t *node_new(const scan_t *scan, where_t where)
{
    CTASSERT(scan != NULL);

    arena_t *arena = scan_get_arena(scan);
    node_t *node = ARENA_MALLOC(sizeof(node_t), "node", scan, arena);
    node->scan = scan;
    node->where = where;

    return node;
}

USE_DECL
const scan_t *node_get_scan(const node_t *node)
{
    CTASSERT(node != NULL);

    return node->scan;
}

USE_DECL
where_t node_get_location(const node_t *node)
{
    CTASSERT(node != NULL);

    return node->where;
}
