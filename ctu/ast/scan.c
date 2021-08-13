#include "scan.h"

#include "ctu/util/util.h"

#include <string.h>

#ifndef _WIN32
#   include <sys/mman.h>
#endif

static scan_t *scan_new(const char *path, size_t size) {
    scan_t *scan = ctu_malloc(sizeof(scan_t));

    scan->path = path;
    scan->data = NULL;

    /* scan->text is filled in by the caller */
    scan->offset = 0;
    scan->size = size;

    return scan;
}

static size_t file_size(FILE *fd) {
    fseek(fd, 0, SEEK_END);
    size_t size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    return size;
}

static const void *map_file(size_t size, FILE *file) {
    char *text;

#ifndef _WIN32
    int fd = fileno(file);
    text = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (text == MAP_FAILED) {
        text = NULL;
    }
#else
    text = ctu_malloc(size + 1);
    fread(text, size, 1, file);
    text[size] = '\0';
#endif

    return text;
}

scan_t *scan_string(const char *path, const char *text) {
    size_t size = strlen(text);
    scan_t *scan = scan_new(path, size);

    scan->text = text;

    return scan;
}

scan_t *scan_file(const char *path, FILE *fd) {
    size_t size = file_size(fd);
    scan_t *scan = scan_new(path, size);

    if (!(scan->text = map_file(size, fd))) {

    }

    return scan;
}

location_t nowhere = { 0, 0, 0, 0 };
