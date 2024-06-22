#pragma once

#include "platform/file.h"

typedef struct reports_t reports_t;

file_t check_open(reports_t *reports, const char *path, file_flags_t mode);
