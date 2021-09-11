#include "ctu/util/util.h"

static const char *VERSION = "0.0.1";

static const driver_t *DRIVER = NULL;

static const driver_t *select_driver(const char *name) {
    if (name == NULL) {
        report2(errors, ERROR, NULL, "No driver specified");
        return &INVALID;
    }

    if (strcmp(name, "pl0") == 0) {
        return &PL0;
    } else if (strcmp(name, "ctu") == 0) {
        return &CTU;
    } else if (strcmp(name, "c") == 0) {
        return &C;
    } else {
        report2(errors, ERROR, NULL, "Unknown driver: %s", name);
        return &INVALID;
    }
}

static const driver_t *driver_for(file_t *file) {
    if (DRIVER != NULL) {
        return DRIVER;
    }

    const char *path = file->path;

    if (endswith(path, ".c")) {
        return &C;
    } else if (endswith(path, ".pl0")) {
        return &PL0;
    } else if (endswith(path, ".ct")) {
        return &CTU;
    } else {
        return &INVALID;
    }
}

static void print_help(const char *name) {
    printf("Cthulhu Compiler Collection\n");
    printf("Usage: %s [options...] [sources...]\n", name);
    printf("Options:\n");
    printf("\t -h, --help: Print this help message\n");
    printf("\t -v, --version: Print version information\n");
    printf("\t -src, --source: Override file extension based compiler choice\n");
    printf("\t -t, --threads: Provide a number of threads to use for parralel compilation\n");

    exit(0);
}

static void print_version(void) {
    printf("Cthulhu Compiler Collection\n");
    printf("Version: %s\n", VERSION);
    printf("Cthulhu Version: %s\n", CTU.version);
    printf("PL/0 Version: %s\n", PL0.version);
    printf("C Version: %s\n", C.version);

    exit(0);
}
