#include "cthulhu/ast/scan.h"

size_t scan_size(const scan_t *scan) {
    return scan->source.size;
}

const char *scan_text(const scan_t *scan) {
    return scan->source.text;
}
