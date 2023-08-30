#pragma once

#include "scan/node.h"

typedef struct obr_string_t {
    char *text;
    size_t length;
} obr_string_t;

obr_string_t obr_parse_string(scan_t *scan, const char *str, size_t length);

#define OBRLTYPE where_t
