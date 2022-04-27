#pragma once

#include "cthulhu/loader/loader.h"

/**
 * internal layout of a saved file
 *
 * header:
 *  - basic header
 *  - count array for each type
 *  - offset array for each type
 *  - string table
 *  - array table
 *  - data tables
 */

#define DIGIT_BASE 26
#define FILE_MAGIC 0xB00B

BEGIN_PACKED(2)

typedef struct PACKED(2) {
    magic_t magic;
    submagic_t submagic;
    semver_t semver;
    offset_t strings;
    offset_t arrays;
} basic_header_t;

END_PACKED

size_t layout_size(layout_t layout);

void begin_data(data_t *data, header_t header);
void end_data(data_t *data);
bool is_loadable(const char *path, uint32_t submagic, uint32_t version);
