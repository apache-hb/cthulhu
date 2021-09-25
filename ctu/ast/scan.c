#include "scan.h"

where_t nowhere = { 0, 0, 0, 0 };

char *ctu_intern(scan_t *scan, char *str) {
    return set_add(scan->pool, str);
}
