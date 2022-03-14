#include "common.h"

hlir_t *load_module(reports_t *reports, const char *path) {
    file_t file = ctu_fopen(path, "rb");
    if (!file_valid(&file)) { return NULL; }

    header_t header;
    fread(&header, sizeof(header), 1, file.file);
    if (header.magic != HEADER_MAGIC) {
        message_t *id = report(reports, ERROR, NULL, "corrupted or invalid ir file: %s", path);
        report_note(id, "file magic was `%x`, expected `%x`", header.magic, HEADER_MAGIC);
        return NULL;
    }

    if (VERSION_MAJOR(header.version) > VERSION_MAJOR(CURRENT_VERSION)) {
        message_t *id = report(reports, ERROR, NULL, "unsupported ir version: %s", path);
        report_note(id, "file version was `%d.%d`, loader only supports up to `%d.%d`", 
            VERSION_MAJOR(header.version), 
            VERSION_MINOR(header.version),
            VERSION_MAJOR(CURRENT_VERSION),
            VERSION_MINOR(CURRENT_VERSION)
        );
        return NULL;
    }

    logverbose(
        "loaded ir file: %s\n"
        " - magic: %x\n"
        " - version: %d.%d.%d\n"
        " - strings: %d\n"
        " - spans: %d",
        path, header.magic,
        VERSION_MAJOR(header.version), VERSION_MINOR(header.version), VERSION_PATCH(header.version),
        header.strings,
        header.spans
    );

    logverbose("counters:");
    for (int i = 0; i < HLIR_TOTAL; i++) {
        logverbose(" counter[%d] %d", i, header.counts[i] - sizeof(header_t));
    }

    return NULL;
}
