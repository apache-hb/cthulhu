#pragma once

#include "cthulhu/hlir2/h2.h"

typedef struct reports_t reports_t;
typedef struct node_t node_t;
typedef struct h2_t h2_t;

const char *h2_to_string(const h2_t *self);

const node_t *h2_get_node(const h2_t *self);
const char *h2_get_name(const h2_t *self);
h2_kind_t h2_get_kind(const h2_t *self);
const h2_t *h2_get_type(const h2_t *self);
const h2_attrib_t *h2_get_attrib(const h2_t *self);
const h2_t *h2_follow_type(const h2_t *self);

bool h2_is(const h2_t *self, h2_kind_t kind);

bool h2_has_quals(const h2_t *self, h2_quals_t quals);
bool h2_has_vis(const h2_t *self, h2_visible_t visible);
