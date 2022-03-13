#include "common.h"

void save_module(reports_t *reports, hlir_t *module, const char *path) {
    UNUSED(module);
    
    file_t file = ctu_fopen(path, "wb");
    if (!file_valid(&file)) { 
        report(reports, ERROR, NULL, "could not open file: %s (errno %d)", path, errno);
        return; 
    }

    header_t header = {
        .magic = HEADER_MAGIC,
        .version = CURRENT_VERSION,
        .symbols = 0,
        .checksum = 0
    };

    fwrite(&header, sizeof(header), 1, file.file);
}
