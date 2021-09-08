#include "ctu/util/util.h"

#include "ctu/frontend/pl0/driver.h"
#include "ctu/frontend/ctu/driver.h"
#include "ctu/frontend/c/driver.h"

static const char *VERSION = "0.0.1";

typedef void*(*parse_t)(file_t*);
typedef lir_t*(*analyze_t)(void*);

typedef struct {
    const char *version;
    const char *name;
    void*(*parse)(file_t*);
    lir_t*(*analyze)(void*);
} driver_t;

static const driver_t PL0 = {
    .version = "0.0.1",
    .name = "PL/0",
    .parse = (parse_t)pl0_parse,
    .analyze = (analyze_t)pl0_analyze
};

static const driver_t CTU = {
    .version = "0.0.1",
    .name = "Cthulhu",
    .parse = (parse_t)ctu_parse,
    .analyze = (analyze_t)ctu_analyze
};

static const driver_t C = {
    .version = "0.0.1",
    .name = "C",
    .parse = (parse_t)c_parse,
    .analyze = (analyze_t)c_analyze
};

static const driver_t INVALID = {
    .version = "1.0.0",
    .name = "Invalid",
    .parse = NULL,
    .analyze = NULL
};

static const driver_t *DRIVER = NULL;

static const driver_t *select_driver(const char *name) {
    if (name == NULL) {
        report(ERROR, "No driver specified");
        return &INVALID;
    }

    if (strcmp(name, "pl0") == 0) {
        return &PL0;
    } else if (strcmp(name, "ctu") == 0) {
        return &CTU;
    } else if (strcmp(name, "c") == 0) {
        return &C;
    } else {
        report(ERROR, "Unknown driver: %s", name);
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
