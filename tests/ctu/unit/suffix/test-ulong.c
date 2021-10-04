
// unit test for ulong suffix

#include <inttypes.h>

#include "ctu/ast/ast.h"
#include "ctu/util/report.h"
#include "ctu/util/str.h"

static const char *signof(mpz_t it) {
    int sign = mpz_sgn(it);
    if (sign > 0) { return ">0"; }
    else if (sign < 0) { return "<0"; }
    else { return "=0"; }
}

int main(void) {
    report_begin(20, true);

    types_init();

    node_t *base10 = ast_digit(NULL, NOWHERE, ctu_strdup("100ul"), 10);
    node_t *base2 = ast_digit(NULL, NOWHERE, ctu_strdup("100ul"), 2);
    node_t *base16 = ast_digit(NULL, NOWHERE, ctu_strdup("100ul"), 16);

    ASSERT(mpz_cmp_ui(base10->num, 100) == 0)("base10 digit parse failed, got `%s`", mpz_get_str(NULL, 10, base10->num));
    ASSERT(base10->sign == false)("base10 sign incorrect, got %s", signof(base10->num));
    ASSERT(base10->integer == INTEGER_LONG)("base10 type incorrect, got `%d`", base10->integer);

    ASSERT(mpz_cmp_ui(base2->num, 0b100) == 0)("base2 digit parse failed, got `%s`", mpz_get_str(NULL, 10, base2->num));
    ASSERT(base2->sign == false)("base2 sign incorrect, got %s", signof(base2->num));
    ASSERT(base2->integer == INTEGER_LONG)("base2 type incorrect, got `%d`", base2->integer);

    ASSERT(mpz_cmp_ui(base16->num, 0x100) == 0)("base16 digit parse failed, got `%s`", mpz_get_str(NULL, 10, base16->num));
    ASSERT(base16->sign == false)("base16 sign incorrect, got %s", signof(base16->num));
    ASSERT(base16->integer == INTEGER_LONG)("base16 type incorrect, got `%d`", base16->integer);

    return report_end("test");
}
    