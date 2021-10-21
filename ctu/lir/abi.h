#pragma once

#include "ctu/type/type.h"

/**
 * mangle a function signature given its type, name, and namespace
 * 
 * @see https://itanium-cxx-abi.github.io/cxx-abi/abi.html
 * 
 * @param type the function type
 * @param parts the function name and namespace
 * 
 * @return the mangled function signature
 */
char *mangle_name(vector_t *parts, const type_t *type);
