#include "scanner.h"

#include "ctu/util/util.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

where_t merge_locations(where_t begin, where_t end) {
    where_t out = {
        begin.first_line,
        begin.first_column,
        end.last_line,
        end.last_column
    };
    return out;
}

void flex_init(where_t *where, int line) {
    where->first_line = line;
    where->last_line = line;
}

int flex_get(scanner_t *scanner, char *out, int size) {
    size_t total = MIN(scanner->size - scanner->offset, (size_t)size);
    
    
    memcpy(out, scanner->text + scanner->offset, total);
    
    scanner->offset += total;

    return total;
}

void flex_update(where_t *where, const char *text) {
    where->first_line = where->last_line;
    where->first_column = where->last_column;

    for (loc_t i = 0; text[i]; i++) {
        if (text[i] == '\n') {
            where->last_line += 1;
            where->last_column = 0;
        } else {
            where->last_column += 1;
        }
    }
}

void free_scanner(scanner_t *scanner) {
    ctu_free(scanner->text);
    ctu_free(scanner);
}
