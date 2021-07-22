
// unit test for intmax suffix

#include <inttypes.h>

#include "ctu/ast/ast.h"
#include "ctu/util/report.h"
#include "ctu/util/str.h"

int main(void) {
    report_begin(20, true);

    node_t *base10 = ast_digit(NULL, NOWHERE, ctu_strdup("100m"), 10);
    node_t *base2 = ast_digit(NULL, NOWHERE, ctu_strdup("100m"), 2);
    node_t *base16 = ast_digit(NULL, NOWHERE, ctu_strdup("100m"), 16);

    ASSERT(base10->digit == 100)("base10 digit parse failed, got `%" PRIu64 "`", base10->digit);
    ASSERT(base10->sign == true)("base10 sign incorrect, got %s", base10->sign ? "true" : "false");
    ASSERT(base10->integer == INTEGER_INTMAX)("base10 type incorrect, got `%d`", base10->integer);

    ASSERT(base2->digit == 0b100)("base2 digit parse failed, got `%" PRIu64 "`", base2->digit);
    ASSERT(base2->sign == true)("base2 sign incorrect, got %s", base2->sign ? "true" : "false");
    ASSERT(base2->integer == INTEGER_INTMAX)("base2 type incorrect, got `%d`", base2->integer);

    ASSERT(base16->digit == 0x100)("base16 digit parse failed, got `%" PRIu64 "`", base16->digit);
    ASSERT(base16->sign == true)("base16 sign incorrect, got %s", base16->sign ? "true" : "false");
    ASSERT(base16->integer == INTEGER_INTMAX)("base16 type incorrect, got `%d`", base16->integer);

    return report_end("test");
}
    