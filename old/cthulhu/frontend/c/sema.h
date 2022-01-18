#pragma once

#include "cthulhu/util/util.h"

typedef struct {
    map_t *funcs;
    map_t *vars;
    map_t *structs;
    map_t *unions;
    map_t *enums;
    map_t *typedefs;
} c_data_t;

c_data_t c_data_new(void);
