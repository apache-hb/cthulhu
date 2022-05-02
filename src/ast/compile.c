#define COMPILER_SOURCE 1

#include "cthulhu/ast/compile.h"
#include "cthulhu/ast/interop.h"
#include "cthulhu/util/file.h"

#include "cthulhu/util/macros.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/util.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static scan_t scan_new(reports_t *reports, const char *language, const char *path)
{
    scan_t scan = {.language = language, .path = path, .reports = reports};

    return scan;
}

scan_t scan_string(reports_t *reports, const char *language, const char *path, const char *text)
{
    text_t source = {.size = strlen(text), .text = text};
    scan_t scan = scan_new(reports, language, path);

    scan.source = source;

    return scan;
}

scan_t scan_file(reports_t *reports, const char *language, file_t file)
{
    error_t error = 0;
    size_t size = file_size(file, &error);
    const char *text = file_map(file, &error);
    scan_t scan = scan_new(reports, language, file.path);
    text_t source = {.size = size, .text = text};

    if (text == NULL || error != 0)
    {
        report(reports, ERROR, NULL, "failed to map file: %s", error_string(error));
    }

    scan.data = BOX(file);
    scan.source = source;

    return scan;
}

scan_t scan_without_source(reports_t *reports, const char *language, const char *path)
{
    scan_t scan = scan_new(reports, language, path);
    text_t source = {.size = 0, .text = ""};
    scan.source = source;
    return scan;
}

void scan_set(scan_t *scan, void *data)
{
    scan->data = data;
}

void *scan_get(scan_t *scan)
{
    return scan->data;
}

void *compile_string(scan_t *scan, callbacks_t *callbacks)
{
    int err;
    void *scanner;
    void *state;

    if ((err = callbacks->init(scan, &scanner)))
    {
        ctu_assert(scan->reports, "failed to init parser for %s: %d", scan->path, err);
        return NULL;
    }

    if (!(state = callbacks->scan(scan_text(scan), scanner)))
    {
        report(scan->reports, ERROR, NULL, "failed to scan %s", scan->path);
        return NULL;
    }

    if ((err = callbacks->parse(scan, scanner)))
    {
        report(scan->reports, ERROR, NULL, "failed to parse %s: %d", scan->path, err);
        return NULL;
    }

    callbacks->destroyBuffer(state, scanner);
    callbacks->destroy(scanner);

    return scan->data;
}

void *compile_file(scan_t *scan, callbacks_t *callbacks)
{
    FILE *fd = scan->data;

    int err;
    void *state;

    if ((err = callbacks->init(scan, &state)))
    {
        return NULL;
    }

    callbacks->setIn(fd, state);

    if ((err = callbacks->parse(scan, state)))
    {
        return NULL;
    }

    callbacks->destroy(state);

    return scan->data;
}
