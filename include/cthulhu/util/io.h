#pragma once

#include "macros.h"
#include "util.h"

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#ifdef CTU_WINDOWS
#   define PATH_SEP "\\"
#else
#   define PATH_SEP "/"
#endif

char *ctu_noext(const char *path) NONULL;
char *ctu_filename(const char *path) NONULL;

struct file_t;

typedef size_t(*file_read_t)(struct file_t *self, void *dst, size_t total);
typedef size_t(*file_write_t)(struct file_t *self, const void *src, size_t total);
typedef size_t(*file_seek_t)(struct file_t *self, size_t offset);
typedef size_t(*file_size_t)(struct file_t *self);
typedef size_t(*file_tell_t)(struct file_t *self);
typedef void*(*file_map_t)(struct file_t *self);
typedef bool(*file_ok_t)(struct file_t *self);

typedef struct {
    file_read_t read;
    file_write_t write;
    file_seek_t seek;
    file_size_t size;
    file_tell_t tell;
    file_map_t mapped;
    file_ok_t ok;
} file_ops_t;

typedef enum {
    BINARY,
    TEXT
} contents_t;

typedef enum {
    READ,
    WRITE,
    EXEC
} access_t;

typedef enum {
    FD, 
    MEMORY
} file_type_t;

typedef struct file_t {
    const char *path;
    contents_t format;
    access_t access;
    file_type_t backing;

    const file_ops_t *ops;
    char data[];
} file_t;

void close_file(file_t *file) NONULL;

file_t *file_new(const char *path, contents_t format, access_t access) NONULL ALLOC(close_file);
file_t *memory_new(const char *name, size_t size, contents_t format, access_t access) NONULL ALLOC(close_file);

size_t file_read(file_t *file, void *dst, size_t total) NONULL;
size_t file_write(file_t *file, const void *src, size_t total) NONULL;
size_t file_seek(file_t *file, size_t offset) NONULL;
size_t file_size(file_t *file) NONULL;
size_t file_tell(file_t *file) NONULL;
void *file_map(file_t *file) NONULL;
bool file_ok(file_t *file) CONSTFN NONULL;

bool file_exists(const char *path) NONULL;
