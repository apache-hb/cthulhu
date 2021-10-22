#include "scan.h"

where_t nowhere = { 0, 0, 0, 0 };

size_t scan_size(const scan_t *scan) {
    return scan->source.size;
}

const char *scan_text(const scan_t *scan) {
    return scan->source.text;
}

char *scan_path(const scan_t *scan) {
    return path_relative(scan->path);
}
