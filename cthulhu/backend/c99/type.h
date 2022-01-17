#pragma once

#include "cthulhu/type/type.h"
#include "cthulhu/gen/value.h"
#include "cthulhu/util/report.h"

/**
 * create an initializer for a type with an identifier
 * 
 * this function takes the variable name due to some of Cs
 * more terse type syntax such as `int x[10]` for arrays and `void(*name)(void)`
 * for function pointers.
 * 
 * @param reports the report sink
 * @param type the type of the variable 
 * @param name the name of the variable being initialized, or NULL
 * 
 * @return the initializer for the type
 */
const char *type_to_string(reports_t *reports, const type_t *type, const char *name);
const char *value_to_string(reports_t *reports, const value_t *value);
