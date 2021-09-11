#include "cmd.h"

#include "ctu/util/util.h"

static const char *VERSION = "0.0.1";
static const char *NAME = "driver";

static void print_help(void) {
    printf("Cthulhu Compiler Collection\n");
    printf("Usage: %s [options...] [sources...]\n", NAME);
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

static void set_threadnum(settings_t *settings, const char *arg) {
    if (settings->threads != 0) {
        report2(settings->reports, WARNING, NULL, "threadnum already set to %zu", settings->threads);
        return;
    }

    if (arg == NULL) {
        report2(settings->reports, ERROR, NULL, "missing argument for --threads");
        return;
    }

    settings->threads = strtoull(arg, NULL, 10);

    if (settings->threads == 0) {
        report2(settings->reports, ERROR, NULL, "invalid argument for --threads: %s", arg);
    }
}

#define MATCH(arg, a, b) (startswith(arg, a) || startswith(arg, b))
#define NEXT(idx, argc, argv) (idx + 1 >= argc ? NULL : argv[idx + 1])

static int parse_arg(settings_t *settings, int index, int argc, char **argv) {
    const char *arg = argv[index];
    
    if (!startswith(arg, "-")) {
        file_t *fp = ctu_open(arg, "rb");

        if (fp == NULL) {
            report2(settings->reports, ERROR, NULL, "failed to open file: %s", arg);
        } else {
            vector_push(&settings->sources, fp);
        }
    } else if (MATCH(arg, "-h", "--help")) {
        print_help();
    } else if (MATCH(arg, "-v", "--version")) {
        print_version();
    } else if (MATCH(arg, "-src", "--source")) {
        if (settings->driver != NULL) {
            report2(settings->reports, ERROR, NULL, "source already specified");
        }

        settings->driver = select_driver(settings->reports, NEXT(index, argc, argv));
        return 2;
    } else if (MATCH(arg, "-t", "--threads")) {
        set_threadnum(settings, NEXT(index, argc, argv));
        return 2;
    } else if (MATCH(arg, "-V", "--verbose")) {
        verbose = true;
    } else {
        report2(settings->reports, WARNING, NULL, "unknown argument %s", arg);
    }

    return 1;
}

settings_t parse_args(reports_t *reports, int argc, char **argv) {
    NAME = argv[0];

    settings_t settings = { 
        .threads = 0, 
        .driver = NULL, 
        .sources = vector_new(0),
        .reports = reports
    };

    if (argc == 1) {
        print_help();
    }

    for (int i = 1; i < argc;) {
        i += parse_arg(&settings, i, argc, argv);
    }

    return settings;
}

