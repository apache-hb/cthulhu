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

typedef struct {
    const char *path;
    FILE *file;
} file_t;

void ctu_close(file_t *fp) NONULL;
file_t ctu_fopen(const char *path, const char *mode) NONULL;
bool file_valid(file_t *fp) CONSTFN NONULL;
size_t file_size(FILE *fd);

size_t ctu_read(void *dst, size_t total, file_t *fp) NONULL;
void *ctu_mmap(file_t *fp) NONULL;

char *ctu_basepath(const char *path) NONULL;
char *ctu_noext(const char *path) NONULL;
char *ctu_filename(const char *path) NONULL;
char *ctu_pathjoin(const char *path, const char *file) NONULL;
char *ctu_realpath(const char *path) NONULL;

struct ctu_file_t;

typedef size_t(*file_read_t)(struct ctu_file_t *self, void *dst, size_t total);
typedef size_t(*file_write_t)(struct ctu_file_t *self, const void *src, size_t total);
typedef size_t(*file_seek_t)(struct ctu_file_t *self, size_t offset);
typedef size_t(*file_size_t)(struct ctu_file_t *self);
typedef size_t(*file_tell_t)(struct ctu_file_t *self);
typedef void*(*file_map_t)(struct ctu_file_t *self);
typedef bool(*file_ok_t)(struct ctu_file_t *self);

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

typedef struct ctu_file_t {
    const char *path;
    contents_t format;
    access_t access;
    file_type_t backing;

    const file_ops_t *ops;
    char data[];
} ctu_file_t;

void close_file(ctu_file_t *file) NONULL;

ctu_file_t *file_new(const char *path, contents_t format, access_t access) NONULL ALLOC(ctu_close);
ctu_file_t *memory_new(const char *name, size_t size, contents_t format, access_t access) NONULL ALLOC(ctu_close);

size_t file_read(ctu_file_t *file, void *dst, size_t total) NONULL;
size_t file_write(ctu_file_t *file, const void *src, size_t total) NONULL;
size_t file_seek(ctu_file_t *file, size_t offset) NONULL;
size_t _file_size(ctu_file_t *file) NONULL;
size_t file_tell(ctu_file_t *file) NONULL;
void *file_map(ctu_file_t *file) NONULL;
bool file_ok(ctu_file_t *file) CONSTFN NONULL;
