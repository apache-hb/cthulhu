#include "scan/node.h"

#include "base/panic.h"
#include "arena/arena.h"

typedef struct node_t
{
    const scan_t *scan;  ///< the source file
    where_t where; ///< the location of this node in the source file
} node_t;

const where_t kNowhere = { 0, 0, 0, 0 };

USE_DECL
node_t *node_builtin(const char *name, arena_t *arena)
{
    CTASSERT(name != NULL);
    CTASSERT(arena != NULL);

    scan_t *scan = scan_builtin(name, arena);
    node_t *node = node_new(scan, kNowhere);

    return node;
}

USE_DECL
bool node_is_builtin(const node_t *node)
{
    CTASSERTF(node != NULL, "node cannot be NULL");
    return scan_is_builtin(node->scan);
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
