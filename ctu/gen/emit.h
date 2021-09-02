#pragma once

#include "ctu/util/util.h"
#include "ctu/type/type.h"
#include "ctu/lir/lir.h"

typedef struct {
    type_t *type;
} value_t;

typedef struct {

} step_t;

typedef struct {
    const char *name;
    value_t *value;

    size_t len;
    size_t size;
    step_t *steps;
} flow_t;

typedef struct {
    const char *name;

    vector_t *flows;
} module_t;

module_t *module_build(lir_t *root);
