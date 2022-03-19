#pragma once

#include "cthulhu/loader/loader.h"

#define DIGIT_BASE 26

size_t layout_size(layout_t layout);

void begin_data(data_t *data, reports_t *reports, const format_t *format, const char *path);
