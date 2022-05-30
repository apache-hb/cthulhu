#define COMPILER_SOURCE 1

#include "cthulhu/ast/compile.h"
#include "cthulhu/ast/interop.h"
#include "platform/file.h"

#include "cthulhu/report/report.h"

#include <string.h>

static scan_t scan_new(reports_t *reports, const char *language, const char *path)
{
    scan_t scan = {
        .language = language,
        .path = path,
        .reports = reports,
    };

    return scan;
}

scan_t scan_string(reports_t *reports, const char *language, const char *path, const char *text)
{
    text_t source = {
        .size = strlen(text),
        .text = text,
    };
    scan_t scan = scan_new(reports, language, path);

    scan.source = source;

    return scan;
}

scan_t scan_file(reports_t *reports, const char *language, file_t file)
{
    cerror_t error = 0;
    size_t size = file_size(file, &error);
    const char *text = file_map(file, &error);
    scan_t scan = scan_new(reports, language, file.path);
    text_t source = {.size = size, .text = text};

    if (text == NULL || error != 0)
    {
        report(reports, ERROR, node_invalid(), "failed to map file: %s", error_string(error));
    }

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

void *compile_string(scan_t *extra, callbacks_t *callbacks)
{    
    int err = 0;
    void *scanner = NULL;
    void *state = NULL;

    if ((err = callbacks->init(extra, &scanner)))
    {
        ctu_assert(extra->reports, "failed to init parser for %s: %d", extra->path, err);
        return NULL;
    }

    if (!(state = callbacks->scan(scan_text(extra), scan_size(extra), scanner)))
    {
        report(extra->reports, ERROR, node_invalid(), "failed to scan %s", extra->path);
        return NULL;
    }

    if ((err = callbacks->parse(scanner, extra)))
    {
        report(extra->reports, ERROR, node_invalid(), "failed to parse %s: %d", extra->path, err);
        return NULL;
    }

    callbacks->destroyBuffer(state, scanner);
    callbacks->destroy(scanner);

    return scan_get(extra);
}
