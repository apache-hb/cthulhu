#include "ctu/util/util.h"

#include "ctu/frontend/pl0/driver.h"
#include "ctu/frontend/ctu/driver.h"
#include "ctu/frontend/c/driver.h"

static const char *VERSION = "0.0.1";

typedef struct {
    const char *version;
    const char *name;
    node_t*(*driver)(file_t*);
} driver_t;

static const driver_t PL0 = {
    .version = "0.0.1",
    .name = "PL/0",
    .driver = pl0_driver
};

static const driver_t CTU = {
    .version = "0.0.1",
    .name = "Cthulhu",
    .driver = ctu_driver
};

static const driver_t C = {
    .version = "0.0.1",
    .name = "C",
    .driver = c_driver
};

static const driver_t INVALID = {
    .version = "1.0.0",
    .name = "Invalid",
    .driver = NULL
};

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
