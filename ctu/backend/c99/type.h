#pragma once

#include "ctu/type/type.h"
#include "ctu/gen/value.h"

/**
 * create an initializer for a type with an identifier
 * 
 * this function takes the variable name due to some of Cs
 * more terse type syntax such as `int x[10]` for arrays and `void(*name)(void)`
 * for function pointers.
 * 
 * @param type the type of the variable 
 * @param name the name of the variable being initialized, or NULL
 * 
 * @return the initializer for the type
 */
const char *type_to_string(const type_t *type, const char *name);
const char *value_to_string(const value_t *value);
