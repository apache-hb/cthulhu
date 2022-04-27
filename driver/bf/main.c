#include "cthulhu/driver/driver.h"

#include "cthulhu/hlir/decl.h"

static const hlir_t *kCursor = NULL;
static const hlir_t *kCell = NULL;
static const hlir_t *kTape = NULL;

void *bf_parse(reports_t *reports, scan_t *scan) {
    UNUSED(reports);
    UNUSED(scan);

    hlir_t *tape = hlir_new_global(NULL, "tape", kTape, NULL);
    hlir_t *cursor = hlir_new_global(NULL, "cursor", kCursor, hlir_int_literal(NULL, kCursor, 0));

    where_t where = { 0, 0, 0, 0 };
    node_t *node = node_new(scan, where);

    vector_t *stmts = vector_new(scan_size(scan));

    const char *text = scan_text(scan);
    while (true) {
        hlir_t *hlir = NULL;
        switch (*text++) {
        case '>':
            hlir = hlir_assign(node, cursor, hlir_add(node, cursor, hlir_int_literal(node, kCursor, 1)));
            break;
        case '<':
            hlir = hlir_assign(node, cursor, hlir_sub(node, cursor, hlir_int_literal(node, kCursor, 1)));
            break;
        case '+':
        case '-':
        case '.':
        case ',':
        case '[':
        case ']':

        default:
            /* everything else is a comment */
            break;
        }

        if (hlir != NULL) {
            vector_push(&stmts, hlir);
        }
    }

    hlir_t *body = hlir_stmts(node, stmts);

    signature_t signature = { .params = vector_new(0), .result = hlir_void(node, "void"), .variadic = false };
    hlir_t *entry = hlir_function(node, "main", signature, vector_of(0), body);

    return NULL;
}

static hlir_t *bf_sema(reports_t *reports, void *ast) {
    UNUSED(reports);

    return ast;
}

static const driver_t kDriver = {
    .name = "brainfuck",
    .version = "1.0.0",
    .parse = bf_parse,
    .sema = bf_sema,
};

int main(int argc, const char **argv) {
    common_init();

    kCursor = hlir_digit(NULL, "size", DIGIT_SIZE, SIGN_UNSIGNED);
    const hlir_t *length = hlir_int_literal(NULL, kCursor, 0x4000);

    kCell = hlir_digit(NULL, "cell", DIGIT_CHAR, SIGN_UNSIGNED);
    kTape = hlir_array(NULL, "tape", kCell, length);

    return common_main(argc, argv, kDriver);
}
