#include "driver.h"

#include "ctu/util/report.h"

#include "scan.h"
#include "sema.h"

static FILE *open_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        report(ERROR, "failed to open file `%s` for parsing", path);
    }
    return fp;
}

node_t *pl0_driver(vector_t *files) {
    size_t len = vector_len(files);
    for (size_t i = 0; i < len; i++) {
        const char *path = vector_get(files, i);
        FILE *fp = open_file(path);
        if (fp != NULL) {
            node_t *program = pl0_compile(path, fp);
            if (program != NULL) {
                pl0_sema(program);
                return program;
            }
        }
    }
    return NULL;
}
