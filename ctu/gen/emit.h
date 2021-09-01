#pragma once

#include "ctu/util/util.h"
#include "ctu/type/type.h"
#include "ctu/lir/lir.h"

typedef struct {

} module_t;

module_t *module_build(lir_t *root);
