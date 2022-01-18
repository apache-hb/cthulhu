#include "cthulhu/driver/driver.h"

#include "cthulhu/util/str.h"
#include "cthulhu/ast/compile.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

static void print_version(driver_t driver) {
    printf("%s: %s\n", driver.name, driver.version);
}

static void print_help(const char **argv) {
    printf("usage: %s <file> [options...]\n", argv[0]);
}

static bool find_arg(int argc, const char **argv, const char *arg) {
    for (int i = 0; i < argc; i++) {
        if (streq(argv[i], arg)) {
            return true;
        }
    }

    return false;
}

int common_main(int argc, const char **argv, driver_t driver) {
    if (find_arg(argc, argv, "--version")) {
        print_version(driver);
        return 0;
    }

    if (find_arg(argc, argv, "--help")) {
        print_help(argv);
        return 0;
    }
    
    reports_t *reports = begin_reports();
    int status = 99;

    if (argc < 2) {
        report(reports, ERROR, NULL, "no file specified");
        return end_reports(reports, SIZE_MAX, "command line parsing");
    }

    const char *path = argv[1];
    file_t file = ctu_fopen(path, "rb");
    if (!file_valid(&file)) {
        report(reports, ERROR, NULL, "failed to open file: %s (%d)", strerror(errno), errno);
        return end_reports(reports, SIZE_MAX, "command line parsing");
    }

    scan_t scan = scan_file(reports, driver.name, &file);
    status = end_reports(reports, SIZE_MAX, "scanning");
    if (status != 0) { return status; }


    void *node = driver.parse(&scan);
    status = end_reports(reports, SIZE_MAX, "parsing");
    if (status != 0) { return status; }
    CTASSERT(node != NULL, "driver.parse == NULL");

    return 0;
}
