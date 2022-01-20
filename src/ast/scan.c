#include "cthulhu/ast/scan.h"

size_t scan_size(const scan_t *scan) {
    return scan->source.size;
}

const char *scan_text(const scan_t *scan) {
    return scan->source.text;
}

scan_t *scan_builtin(const char *language) {
    scan_t *scan = ctu_malloc(sizeof(scan_t));
    scan->language = language;
    scan->path = "builtin";
    scan->source.text = NULL;
    return scan;
}

bool is_builtin_scanner(const scan_t *scan) {
    return scan->source.text == NULL;
}
