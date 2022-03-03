#include "miniz/miniz.h"

#include <stdlib.h>
#include <stdnoreturn.h>

const char *boostrap = 
    "#!/bin/sh\n" // set the shebang
    "set -euo pipefail\n" // set the shell to exit on error and pipefail
    "exec $LAMBDA_TASK_ROOT/function $AWS_LAMBDA_RUNTIME_API" // execute the function with the first argument being the endpoint
;

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *data = fopen(argv[1], "rb");
    fseek(data, 0, SEEK_END);
    size_t size = ftell(data);
    fseek(data, 0, SEEK_SET);

    char *buffer = malloc(size);
    fread(buffer, size, 1, data);
    fclose(data);

    remove("lambda.zip");

    mz_bool status = mz_zip_add_mem_to_archive_file_in_place(
        "lambda.zip", "function", buffer, size, 
        NULL, 0, MZ_BEST_COMPRESSION
    );

    if (!status) {
        fprintf(stderr, "Failed to add lambda to zip\n");
        return 1;
    }

    status = mz_zip_add_mem_to_archive_file_in_place(
        "lambda.zip", "bootstrap", boostrap, strlen(boostrap), 
        NULL, 0, MZ_BEST_COMPRESSION
    );

    if (!status) {
        fprintf(stderr, "Failed to add bootstrap to zip\n");
        return 1;
    }
}
