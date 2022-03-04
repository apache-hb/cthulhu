#include "cthulhu/driver/driver.h"

static const hlir_t *CURSOR = NULL;
static const hlir_t *CELL = NULL;
static const hlir_t *TAPE = NULL;

void *bf_parse(reports_t *reports, scan_t *scan) {
    UNUSED(reports);
    UNUSED(scan);

    hlir_t *tape = hlir_value(NULL, "tape", TAPE, NULL);
    hlir_t *cursor = hlir_value(NULL, "cursor", CURSOR, hlir_int_literal(NULL, CURSOR, 0));

    return NULL;
}

static hlir_t *bf_sema(reports_t *reports, void *ast) {
    UNUSED(reports);

    return ast;
}

static const driver_t DRIVER = {
    .name = "brainfuck",
    .version = "1.0.0",
    .parse = bf_parse,
    .sema = bf_sema
};

int main(int argc, const char **argv) {
    common_init();

    CURSOR = hlir_digit(NULL, "size", DIGIT_SIZE, SIGN_UNSIGNED);
    const hlir_t *length = hlir_int_literal(NULL, CURSOR, 0x4000);

    CELL = hlir_digit(NULL, "cell", DIGIT_CHAR, SIGN_UNSIGNED);
    TAPE = hlir_array(NULL, "tape", CELL, length);

    return common_main(argc, argv, DRIVER);
}
