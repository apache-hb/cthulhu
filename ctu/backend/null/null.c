#include "null.h"

bool null_build(reports_t *reports, module_t *mod, const char *path) {
    (void)mod;
    report2(reports, NOTE, NULL, "using null backend for %s", path);
    return true;
}
