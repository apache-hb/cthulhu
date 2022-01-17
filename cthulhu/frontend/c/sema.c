#include "sema.h"

c_data_t c_data_new(void) {
    c_data_t data = {
        .funcs = map_new(4),
        .vars = map_new(4),
        .structs = map_new(4),
        .enums = map_new(4),
        .typedefs = map_new(4)
    };
    
    return data;
}
