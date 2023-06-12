#pragma once

#include "scan/node.h"

#define CCLTYPE where_t

void cc_init_scan(scan_t *scan);

bool cc_is_typename(scan_t *scan, const char *text);
