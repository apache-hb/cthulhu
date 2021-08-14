#include "scan.h"

static const char *LANGUAGE = "C";

scan_t *c_scan_file(const char *path, FILE *fd) {
    return scan_file(LANGUAGE, path, fd);
}
