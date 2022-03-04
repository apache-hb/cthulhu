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
        fprintf(stderr, "Usage: %s <file> [output]\n", argv[0]);
        return 1;
    }

    const char *file = argv[1];
    const char *output = argc > 2 ? argv[2] : "lambda.zip";

    FILE *data = fopen(file, "rb");
    fseek(data, 0, SEEK_END);
    size_t size = ftell(data);
    fseek(data, 0, SEEK_SET);

    char *buffer = malloc(size);
    fread(buffer, size, 1, data);
    fclose(data);

    remove(output);

    mz_bool status = mz_zip_add_mem_to_archive_file_in_place(
        output, "function", buffer, size, 
        NULL, 0, MZ_BEST_COMPRESSION
    );

    if (!status) {
        fprintf(stderr, "Failed to add lambda to zip\n");
        return 1;
    }

    status = mz_zip_add_mem_to_archive_file_in_place(
        output, "bootstrap", boostrap, strlen(boostrap), 
        NULL, 0, MZ_BEST_COMPRESSION
    );

    if (!status) {
        fprintf(stderr, "Failed to add bootstrap to zip\n");
        return 1;
    }
}
