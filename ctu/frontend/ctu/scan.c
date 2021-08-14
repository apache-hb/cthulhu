#include "scan.h"

static const char *LANGUAGE = "cthulhu";

scan_t *ctu_scan_file(const char *path, FILE *fd) {
    return scan_file(LANGUAGE, path, fd);
}
