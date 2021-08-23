#include "ctu/util/util.h"

#include "ctu/frontend/pl0/driver.h"
#include "ctu/frontend/ctu/driver.h"

static const char *VERSION = "0.0.1";

typedef struct {
    const char *version;
    const char *name;
    vector_t*(*driver)(vector_t*);
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

static const driver_t INVALID = {
    .version = "1.0.0",
    .name = "Invalid",
    .driver = NULL
};

static driver_t select_driver(const char *lang) {
    if (lang == NULL) {
        report(ERROR, "language requires a parameter");
        return INVALID;
    }

    if (strcmp(lang, "ctu") == 0) {
        return CTU;
    } else if (strcmp(lang, "pl0") == 0) {
        return PL0;
    } else {
        report(ERROR, "unknown language `%s`", lang);
        return INVALID;
    }
}

static driver_t select_name(const char *name) {
    if (strstr(name, "ctc") != NULL) {
        return CTU;
    } else if (strstr(name, "pl0") != NULL) {
        return PL0;
    } else {
        return INVALID;
    }
}

static void print_help(const char *name) {
    printf("Cthulhu Compiler Collection\n");
    printf("Usage: %s [options...] [sources...]\n", name);
    printf("Options:\n");
    printf("\t -h, --help: Print this help message\n");
    printf("\t -v, --version: Print version information\n");
    printf("\t -l, --language: Language to compile (ctu, pl0)\n");

    exit(0);
}

static void print_version(void) {
    printf("Cthulhu Compiler Collection\n");
    printf("Version: %s\n", VERSION);
    printf("Cthulhu Version: %s\n", CTU.version);
    printf("PL/0 Version: %s\n", PL0.version);

    exit(0);
}
