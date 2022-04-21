#pragma once

#include "cthulhu/util/io.h"

typedef void(*file_open_t)(ctu_file_t **file, const char *path, contents_t format, access_t access);
typedef void(*file_close_t)(ctu_file_t *file);

void platform_close(ctu_file_t *file) NONULL;
void platform_open(ctu_file_t **file, const char *path, contents_t format, access_t access) NONULL ALLOC(platform_close);

void memory_close(ctu_file_t *file) NONULL;
void memory_open(ctu_file_t **file, const char *name, size_t size, contents_t format, access_t access) NONULL ALLOC(memory_close);
