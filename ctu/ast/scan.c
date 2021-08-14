#include "scan.h"
#include "interop.h"

#include "ctu/util/util.h"
#include "ctu/util/report.h"

#include <errno.h>
#include <string.h>

#ifndef _WIN32
#   include <sys/mman.h>
#endif

static scan_t *scan_new(const char *language, const char *path, size_t size) {
    scan_t *scan = ctu_malloc(sizeof(scan_t));

    scan->language = language;
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
        report(INTERNAL, "failed to mmap file: %s\n", strerror(errno));
    }
#else
    text = ctu_malloc(size + 1);
    fread(text, size, 1, file);
    text[size] = '\0';
#endif

    return text;
}

scan_t *scan_string(const char *language, const char *path, const char *text) {
    size_t size = strlen(text);
    scan_t *scan = scan_new(language, path, size);

    scan->text = text;

    return scan;
}

scan_t *scan_file(const char *language, const char *path, FILE *fd) {
    size_t size = file_size(fd);
    scan_t *scan = scan_new(language, path, size);

    scan->data = fd;

    if (!(scan->text = map_file(size, fd))) {
        end_report("file mapping");
    }

    return scan;
}

void scan_export(scan_t *scan, void *data) {
    scan->data = data;
}

where_t nowhere = { 0, 0, 0, 0 };


void *compile_string(scan_t *extra, callbacks_t *callbacks) {
    int err;
    void *scanner;
    void *state;

    if ((err = callbacks->init(extra, &scanner))) {
        return NULL;
    }

    if (!(state = callbacks->scan(extra->text, scanner))) {
        return NULL;
    }

    if ((err = callbacks->parse(scanner, extra))) {
        return NULL;
    }

    callbacks->destroy(scanner);

    return extra->data;
}

void *compile_file(scan_t *extra, callbacks_t *callbacks) {
    FILE *fd = extra->data;

    int err;
    void *scanner;

    if ((err = callbacks->init(extra, &scanner))) {
        return NULL;
    }

    callbacks->set_in(fd, scanner);

    if ((err = callbacks->parse(scanner, extra))) {
        return NULL;
    }

    callbacks->destroy(scanner);

    return extra->data;
}