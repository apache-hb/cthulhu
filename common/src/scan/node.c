#include "scan/node.h"

#include "base/macros.h"
#include "base/panic.h"

#include <limits.h>

typedef struct
{
    scan_t *scan;  ///< the source file
    where_t where; ///< the location of this node in the source file
} node_data_t;

#define TOTAL_NODES (0x1000 * 64) // TODO: make this configurable

static node_data_t kNodeData[TOTAL_NODES] = {0};
static node_t kNodeOffset = 0;

node_t node_builtin(void)
{
    return UINT_MAX - 1;
}

node_t node_invalid(void)
{
    return UINT_MAX;
}

bool node_is_valid(node_t node)
{
    return node != node_invalid();
}

node_t node_new(scan_t *scan, where_t where)
{
    node_t offset = kNodeOffset++;

    node_data_t *node = kNodeData + offset;
    node->scan = scan;
    node->where = where;

    return offset;
}

scan_t *get_node_scanner(node_t node)
{
    CTASSERTF(node < TOTAL_NODES, "[get-node-scanner] node %u out of range", node);
    const node_data_t *self = kNodeData + node;

    return self->scan;
}

where_t get_node_location(node_t node)
{
    CTASSERTF(node < TOTAL_NODES, "[get-node-location] node %u out of range", node);
    const node_data_t *self = kNodeData + node;

    return self->where;
}
