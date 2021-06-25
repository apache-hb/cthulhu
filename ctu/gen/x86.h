#pragma once

#include <stdint.h>

/**
 * generic codegen for 8086, x86, and x64
 */

/**
 * a single function
 */
typedef struct {
    uint8_t *bytes;
} func_t;

/**
 * a blob of functions 
 * with relocations and extra data
 */
typedef struct {

} blob_t;
