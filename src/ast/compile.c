#define COMPILER_SOURCE 1

#include "cthulhu/util/error.h"
#include "cthulhu/ast/interop.h"
#include "cthulhu/ast/compile.h"

#include "cthulhu/util/util.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/macros.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>

static scan_t scan_new(reports_t *reports, const char *language, const char *path) {
    scan_t scan = {
        .language = language,
        .path = path,
        .reports = reports
    };

    return scan;
}

scan_t scan_string(reports_t *reports, const char *language, const char *path, const char *text) {
    text_t source = { .size = strlen(text), .text = text };
    scan_t scan = scan_new(reports, language, path);

    scan.source = source;

    return scan;
}

scan_t scan_file(reports_t *reports, const char *language, file_t *file) {
    size_t size = file_size(file);
    char *text = file_map(file);
    scan_t scan = scan_new(reports, language, file->path);
    text_t source = { 
        .size = size, 
        .text = text 
    };

    if (text == NULL) {
        ctu_errno_t err = ctu_last_error();
        report(reports, ERROR, NULL, "failed to map file: %s", ctu_err_string(err));
    }

    scan.data = file;
    scan.source = source;

    return scan;
}

void scan_set(scan_t *scan, void *data) {
    scan->data = data;
}

void *scan_get(scan_t *scan) {
    return scan->data;
}

void *compile_string(scan_t *extra, callbacks_t *callbacks) {
    int err;
    void *scanner;
    void *state;

    if ((err = callbacks->init(extra, &scanner))) {
        ctu_assert(extra->reports, "failed to init parser for %s due to %d", extra->path, err);
        return NULL;
    }

    if (!(state = callbacks->scan(scan_text(extra), scanner))) {
        report(extra->reports, ERROR, NULL, "failed to scan %s", extra->path);
        return NULL;
    }

    if ((err = callbacks->parse(scanner, extra))) {
        report(extra->reports, ERROR, NULL, "failed to parse %s", extra->path);
        return NULL;
    }

    callbacks->destroy(scanner);

    return extra->data;
}

void *compile_file(scan_t *scan, callbacks_t *callbacks) {
    FILE *fd = scan->data;

    int err;
    void *state;

    if ((err = callbacks->init(scan, &state))) {
        return NULL;
    }

    callbacks->set_in(fd, state);

    if ((err = callbacks->parse(scan, state))) {
        return NULL;
    }

    callbacks->destroy(state);

    return scan->data;
}
