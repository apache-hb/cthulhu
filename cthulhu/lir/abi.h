#pragma once

#include "cthulhu/type/type.h"
#include "lir.h"

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

/**
 * mangle a function signature given its type, name, and namespace
 * 
 * @see https://itanium-cxx-abi.github.io/cxx-abi/abi.html
 * 
 * @param path the namespace of this symbol
 * @param lir the symbol
 * 
 * @return the mangled function signature
 */
char *mangle_symbol(vector_t *path, const lir_t *lir);
