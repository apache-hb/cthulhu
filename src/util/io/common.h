#pragma once

#include "cthulhu/util/io.h"

typedef void (*file_open_t)(file_t **file, const char *path, contents_t format, access_t access);
typedef void (*file_close_t)(file_t *file);

void platform_close(file_t *file);
void platform_open(file_t **file, const char *path, contents_t format, access_t access);

void memory_close(file_t *file);
void memory_open(file_t **file, const char *name, size_t size, contents_t format, access_t access);
