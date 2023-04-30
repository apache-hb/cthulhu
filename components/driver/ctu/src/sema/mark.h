#pragma once

#include <stdbool.h>

typedef struct map_t map_t;
typedef struct hlir_t hlir_t;


void user_mark_variant(hlir_t *self);

/**
 * @brief set the default case for a variant
 * 
 * @param self 
 * @param it 
 * @return NULL if the default is not already set, otherwise the previous default
 */
const hlir_t *user_set_variant_default(hlir_t *self, const hlir_t *it);

void user_add_variant_case(hlir_t *self, const char *name, const hlir_t *it);

map_t *user_variant_cases(const hlir_t *self);
const hlir_t *user_variant_default_case(const hlir_t *self);
