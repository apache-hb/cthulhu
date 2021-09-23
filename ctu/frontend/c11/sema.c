#include "sema.h"

c_data_t *c_data_new(void) {
    c_data_t *data = NEW(c_data_t);

    data->funcs = map_new(4);
    data->vars = map_new(4);
    data->structs = map_new(4);
    data->unions = map_new(4);
    data->enums = map_new(4);
    data->typedefs = map_new(4);

    return data;
}
