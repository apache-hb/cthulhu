#include "cthulhu/loader/hlir.h"
#include "cthulhu/debug/hlir.h"

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <path>\n", argv[0]);
        return 1;
    }

    reports_t *reports = begin_reports();
    const char *path = argv[1];
    hlir_t *hlir = load_module(reports, path);

    int status = end_reports(reports, SIZE_MAX, "module parsing");
    if (status != 0) { return status; }

    if (hlir != NULL) {
        debug_hlir(reports, hlir);
    }

    return end_reports(reports, SIZE_MAX, "debug output");
}
