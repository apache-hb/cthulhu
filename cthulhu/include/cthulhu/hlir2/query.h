#pragma once

#include "cthulhu/hlir2/h2.h"

typedef struct reports_t reports_t;
typedef struct node_t node_t;
typedef struct h2_t h2_t;

const node_t *h2_get_node(const h2_t *self);
const char *h2_get_name(const h2_t *self);
bool h2_is_const(const h2_t *self);
bool h2_is_visible(const h2_t *self, h2_visible_t visible);
