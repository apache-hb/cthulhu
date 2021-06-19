#include "scanner.h"

#include <string.h>

void flex_init(where_t *where) {
    where->first_line = 1;
    where->first_column = 1;
    where->last_line = 1;
    where->last_column = 1;
}

int flex_get(scanner_t *scanner, char *out) {
    int letter = scanner->next(scanner->file);
    *out = letter;
    return !!letter;
}

void flex_update(where_t *where, int line, int length, const char *text) {
    where->first_line = where->last_line;
    where->first_column = where->last_column;

    if (where->last_line == line) {
        where->last_column += length;
    } else {
        where->last_line = line;
        where->last_column = text + length - strrchr(text, '\n');
    }
}
