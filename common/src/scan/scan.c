#include "scan/scan.h"

#include "platform/file.h"
#include "report/report.h"

#include <string.h>
#include <limits.h>

typedef struct
{
    const char *language;      ///< the language this file contains
    const char *path;          ///< the path to this file
    void *data;                ///< user data pointer
    text_t source;             ///< the source text in this file
    size_t offset;             ///< how much of this file has been parsed
    struct reports_t *reports; ///< the reporting sink for this file
} scan_data_t;

#define TOTAL_SCANNERS (0x1000) // TODO: make this configurable

static scan_data_t kScanData[TOTAL_SCANNERS];
static scan_t kScanOffset = 0;

static scan_data_t *get_scanner(scan_t scan)
{
    CTASSERTF(scan < TOTAL_SCANNERS, "[get-scanner] scan %u out of range", scan);
    return kScanData + scan;
}

const char *scan_language(scan_t scan)
{
    const scan_data_t *self = get_scanner(scan);
    return self->language;
}

const char *scan_path(scan_t scan)
{
    const scan_data_t *self = get_scanner(scan);
    return self->path;
}

void *scan_get(scan_t scan)
{
    const scan_data_t *self = get_scanner(scan);
    return self->data;
}

void scan_set(scan_t scan, void *value)
{
    scan_data_t *self = get_scanner(scan);
    self->data = value;
}

const char *scan_text(scan_t scan)
{
    const scan_data_t *self = get_scanner(scan);
    return self->source.text;
}

text_t scan_source(scan_t scan)
{
    scan_data_t *self = get_scanner(scan);
    return self->source;
}

size_t scan_size(scan_t scan)
{
    const scan_data_t *self = get_scanner(scan);
    return self->source.size;
}

size_t scan_offset(scan_t scan)
{
    const scan_data_t *self = get_scanner(scan);
    return self->offset;
}

void scan_advance(scan_t scan, size_t offset)
{
    scan_data_t *self = get_scanner(scan);
    self->offset += offset;
}

reports_t *scan_reports(scan_t scan)
{
    const scan_data_t *self = get_scanner(scan);
    return self->reports;
}

scan_t scan_invalid(void)
{
    return UINT_MAX;
}


static scan_t scan_new(reports_t *reports, const char *language, const char *path, text_t source)
{
    scan_t index = kScanOffset++;

    scan_data_t *self = get_scanner(index);
    self->language = language;
    self->path = path;
    self->reports = reports;
    self->offset = 0;
    self->source = source;

    return index;
}

scan_t scan_string(reports_t *reports, const char *language, const char *path, const char *text)
{
    text_t source = {
        .size = strlen(text),
        .text = text,
    };
    return scan_new(reports, language, path, source);
}

scan_t scan_file(reports_t *reports, const char *language, file_t file)
{
    cerror_t error = 0;
    size_t size = file_size(file, &error);
    const char *text = file_map(file, &error);
    text_t source = {.size = size, .text = text};

    if (text == NULL || error != 0)
    {
        report(reports, ERROR, node_invalid(), "failed to map file: %s", error_string(error));
    }

    return scan_new(reports, language, file.path, source);
}

scan_t scan_without_source(reports_t *reports, const char *language, const char *path)
{
    text_t source = {.size = 0, .text = ""};
    return scan_new(reports, language, path, source);
}
