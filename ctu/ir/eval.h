#include "ir.h"

typedef struct {
    type_t *type;

    union {
        /* if this is a digit this is active */
        mpz_t digit;

        /* if this is a bool this is active */
        bool boolean;

        /* if this is a string this is active */
        char *string;

        /* if this is a struct or a union this is active */
        map_t *items;

        /* if this is a pointer this points to a value */
        value_t *value;
    };
} value_t;

value_t *eval_global(module_t *mod, flow_t *flow);
