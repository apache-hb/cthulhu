static const char *VERSION = "0.0.1";
static const char *PL0_VERSION = "0.0.1";
static const char *CTU_VERSION = "0.0.1";

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
    printf("Cthulhu Version: %s\n", CTU_VERSION);
    printf("PL/0 Version: %s\n", PL0_VERSION);

    exit(0);
}
