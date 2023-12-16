#include "scan/node.h"

#include "common.h"

#include "base/panic.h"
#include "memory/arena.h"

#include <limits.h>

typedef struct node_t
{
    scan_t *scan;  ///< the source file
    where_t where; ///< the location of this node in the source file
} node_t;

static node_t kBuiltinNode = {
    .scan = NULL,
    .where = { 0, 0, 0, 0 }
};

static node_t kInvalidNode = {
    .scan = NULL,
    .where = { UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX }
};

USE_DECL
node_t *node_builtin(void)
{
    return &kBuiltinNode;
}

USE_DECL
node_t *node_invalid(void)
{
    return &kInvalidNode;
}

USE_DECL
bool node_has_scanner(const node_t *node)
{
    return node != NULL && node->scan != NULL;
}

USE_DECL
bool node_is_invalid(const node_t *node)
{
    return node == node_invalid();
}

USE_DECL
bool node_is_builtin(const node_t *node)
{
    return node == node_builtin();
}

USE_DECL
node_t *node_new(scan_t *scan, where_t where)
{
    CTASSERT(scan != NULL);

    arena_t *alloc = scan_alloc(scan);
    node_t *node = ARENA_MALLOC(alloc, sizeof(node_t), "node", scan);
    node->scan = scan;
    node->where = where;

    return node;
}

USE_DECL
scan_t *node_get_scan(const node_t *node)
{
    CTASSERT(node_has_scanner(node));

    return node->scan;
}

USE_DECL
where_t node_get_location(const node_t *node)
{
    CTASSERT(node != NULL);

    return node->where;
}
