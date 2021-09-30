#include "interop.h"

#include <string.h>

#include "ctu/util/util.h"

void flex_action(where_t *where, const char *text) {
    where->first_line = where->last_line;
    where->first_column = where->last_column;

    for (int64_t i = 0; text[i]; i++) {
        if (text[i] == '\n') {
            where->last_line += 1;
            where->last_column = 0;
        } else {
            where->last_column += 1;
        }
    }
}

int flex_input(scan_t *scan, char *out, int size) {
    int total = MIN((int)(scan_size(scan) - scan->offset), size);

    memcpy(out, scan_text(scan) + scan->offset, total);

    scan->offset += total;

    return total;
}

void flex_init(where_t *where) {
    where->first_line = 0;
    where->last_line = 0;
}
