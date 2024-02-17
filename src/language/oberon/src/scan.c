#include "cthulhu/broker/scan.h"

void obrerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    ctx_error(where, state, scan, msg);
}
