#include "scanner.h"

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

static void scanner_add(scanner_t *scanner, char c) {
    if (scanner->len + 2 > scanner->size) {
        scanner->size += 0x1000;
        scanner->text = realloc(scanner->text, scanner->size);
    }
    scanner->text[scanner->len++] = c;
    scanner->text[scanner->len] = 0;
}

int flex_get(scanner_t *scanner, char *out, int size) {
    int total = 0;

    while (size > 0) {
        int letter;
        while ((letter = scanner->next(scanner->handle)) == '\r');
        
        size -= 1;

        out[total++] = letter;
        if (!letter) {
            break;
        }

        scanner_add(scanner, (char)letter);
    }

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
