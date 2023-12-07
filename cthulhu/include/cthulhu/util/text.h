#pragma once

#include <stddef.h>

typedef struct reports_t reports_t;
typedef struct node_t node_t;

typedef struct util_text_t {
    char *text;
    size_t length;
} util_text_t;

util_text_t util_text_escape(reports_t *reports, const node_t *node, const char *text, size_t length);
