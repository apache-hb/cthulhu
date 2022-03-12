#include "common.h"

hlir_t *load_module(reports_t *reports, const char *path) {
    file_t file = ctu_fopen(path, "rb");
    if (!file_valid(&file)) { return NULL; }

    header_t header;
    fread(&header, sizeof(header), 1, file.file);
    if (header.magic != HEADER_MAGIC) {
        message_t *id = report(reports, ERROR, NULL, "possibly corrupted ir file: %s", path);
        report_note(id, "file magic was `%x`, expected `%x`", header.magic, HEADER_MAGIC);
        return NULL;
    }

    return NULL;
}
