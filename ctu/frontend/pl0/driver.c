#include "driver.h"

#include "ctu/util/report.h"

#include "scan.h"
#include "sema.h"

vector_t *pl0_driver(vector_t *files) {
    size_t len = vector_len(files);
    vector_t *result = vector_new(len);

    for (size_t i = 0; i < len; i++) {
        file_t *fp = vector_get(files, i);

        if (fp != NULL) {
            node_t *program = pl0_compile(fp);
            vector_push(&result, program);
        }
    }

    return result;
}
