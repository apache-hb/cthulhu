#include "value.h"

#include <string.h>

gcc_jit_rvalue *rvalue_from_value(gcc_jit_context *context, const value_t *value) {
    const type_t *vtype = value->type;
    gcc_jit_type *type = select_gcc_type(context, vtype);

    if (is_digit(vtype)) {
        return gcc_jit_context_new_rvalue_from_long(context, type, mpz_get_si(value->digit));
    }

    if (is_bool(vtype)) {
        if (value->boolean) {
            return gcc_jit_context_one(context, type);
        } else {
            return gcc_jit_context_zero(context, type);
        }
    }

    return NULL;
}
