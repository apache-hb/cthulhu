#pragma once

#include "cthulhu/loader/loader.h"

/**
 * internal layout of a saved file
 * 
 * header:
 *  - magic
 *  - submagic
 *  - version
 *  - specific header
 *  - count array for each type
 *  - offset array for each type
 *  - string table
 *  - data tables
 */

#define DIGIT_BASE 26
#define FILE_MAGIC 0xB00B

size_t layout_size(layout_t layout);

void begin_data(
    data_t *data, 
    header_t header
);
