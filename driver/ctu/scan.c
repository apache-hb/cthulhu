#include "scan.h"

#include "cthulhu/util/util.h"
#include "cthulhu/ast/compile.h"

typedef struct
{
    size_t depth; // template depth
} lex_extra_t;

void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}

void init_scan(scan_t *scan)
{
    lex_extra_t *extra = ctu_malloc(sizeof(lex_extra_t));
    extra->depth = 0;
    scan_set(scan, extra);
}

void enter_template(scan_t *scan)
{
    lex_extra_t *extra = scan_get(scan);
    extra->depth++;
}

size_t exit_template(scan_t *scan)
{
    lex_extra_t *extra = scan_get(scan);
    return extra->depth--;
}
