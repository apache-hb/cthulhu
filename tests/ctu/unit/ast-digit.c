// ast unit tests

#include "ctu/ast/ast.h"
#include "ctu/util/str.h"
#include "ctu/util/report.h"

int main(void) {
    report_begin(1, true);
    types_init();
    
    node_t *digit10 = ast_digit(NULL, NOWHERE, ctu_strdup("100"), 10);
    node_t *digit2 = ast_digit(NULL, NOWHERE, ctu_strdup("1000"), 2);
    node_t *digit16 = ast_digit(NULL, NOWHERE, ctu_strdup("fFa6"), 16);

    ASSERT(mpz_cmp_ui(digit10->num, 100) == 0)("parsed base 10 wrong");
    ASSERT(mpz_cmp_ui(digit2->num, 0b1000) == 0)("parsed base 2 wrong");
    ASSERT(mpz_cmp_ui(digit16->num, 0xFFA6) == 0)("parsed base 16 wrong");

    return report_end("test");
}
