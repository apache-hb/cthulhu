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

static scan_t *scan_new(reports_t *reports, const char *language, const char *path, size_t size) {
    scan_t *scan = NEW(scan_t);

    scan->language = language;
    scan->path = path;
    scan->data = NULL;

    /* scan->text is filled in by the caller */
    scan->offset = 0;
    scan->size = size;

    scan->reports = reports;

    return scan;
}

scan_t *scan_string(reports_t *reports, const char *language, const char *path, const char *text) {
    size_t size = strlen(text);
    scan_t *scan = scan_new(reports, language, path, size);

    scan->text = text;

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
    scan_t *scan = scan_new(reports, language, file->path, size);

    scan->data = fd;

    if (!(scan->text = ctu_mmap(file))) {
        assert2(reports, "failed to mmap file");
    }

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
