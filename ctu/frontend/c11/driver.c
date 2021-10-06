#include "driver.h"

#include "ctu/driver/driver.h"

c_t *c_parse(reports_t *reports, file_t *file) {
    report(reports, INTERNAL, NULL, "C is unimplemented %s", file->path);
    return NULL;
}

lir_t *c_analyze(reports_t *reports, c_t *node) {
    report(reports, INTERNAL, node->node, "C is unimplemented");
    return NULL;
}

static const frontend_t DRIVER = {
    .version = "0.0.1",
    .name = "C",
    .parse = (parse_t)c_parse,
    .analyze = (analyze_t)c_analyze
};

int main(int argc, char **argv) {
    return common_main(&DRIVER, argc, argv);
}
