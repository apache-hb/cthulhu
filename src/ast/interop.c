#include "cthulhu/util/macros.h"
#include "cthulhu/util/util.h"


#include <string.h>

#define COMPILER_SOURCE 1

#include "cthulhu/ast/interop.h"

void flex_action(where_t *where, const char *text) {
    where->firstLine = where->lastLine;
    where->firstColumn = where->lastColumn;

    for (int64_t i = 0; text[i]; i++) {
        if (text[i] == '\n') {
            where->lastLine += 1;
            where->lastColumn = 0;
        } else {
            where->lastColumn += 1;
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
    where->firstLine = 0;
    where->lastLine = 0;
}
