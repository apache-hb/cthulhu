#pragma once

#include "ctu/ast/scan.h"

scan_t *c_scan_file(const char *path, FILE *fd);
