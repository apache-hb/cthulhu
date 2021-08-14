#pragma once

#include "ctu/ast/scan.h"

scan_t *ctu_scan_file(const char *path, FILE *fd);
