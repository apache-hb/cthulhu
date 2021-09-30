#include "compile.h"
#include "interop.h"

#include "ctu/util/util.h"
#include "ctu/util/report.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifndef _WIN32
#   include <sys/mman.h>
#endif

static scan_t *scan_new(reports_t *reports, const char *language, const char *path) {
    scan_t *scan = ctu_malloc(sizeof(scan_t));

    scan->language = language;
    scan->path = path;
    scan->data = NULL;

    scan->offset = 0;

    scan->pool = set_new(1257787); /* TODO: figure out a good size based on file size */

    scan->reports = reports;

    return scan;
}

void scan_delete(scan_t *scan) {
    set_delete(scan->pool);
    ctu_free(scan, sizeof(scan_t));
}

scan_t *scan_string(reports_t *reports, const char *language, const char *path, const char *text) {
    scan_t *scan = scan_new(reports, language, path);

    scan->source = (text_t){ strlen(text), text };

    return scan;
}

static size_t file_size(FILE *fd) {
    fseek(fd, 0, SEEK_END);
    size_t size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    return size;
}

scan_t *scan_file(reports_t *reports, const char *language, file_t *file) {
    FILE *fd = file->file;
    size_t size = file_size(fd);
    scan_t *scan = scan_new(reports, language, file->path);

    scan->data = fd;

    char *text;
    if (!(text = ctu_mmap(file))) {
        ctu_assert(reports, "failed to mmap file");
    }

    scan->source = (text_t){ size, text };

    return scan;
}

void scan_export(scan_t *scan, void *data) {
    scan->data = data;
}

void *compile_string(scan_t *extra, callbacks_t *callbacks) {
    int err;
    void *scanner;
    void *state;

    if ((err = callbacks->init(extra, &scanner))) {
        return NULL;
    }

    if (!(state = callbacks->scan(scan_text(extra), scanner))) {
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
