// ast unit tests

#include "ctu/ast/ast.h"
#include "ctu/util/str.h"
#include "ctu/util/report.h"

int main(void) {
    report_begin(1, true);
    
    node_t *digit10 = ast_digit(NULL, NOWHERE, ctu_strdup("100"), 10);
    node_t *digit2 = ast_digit(NULL, NOWHERE, ctu_strdup("1000"), 2);
    node_t *digit16 = ast_digit(NULL, NOWHERE, ctu_strdup("fFa6"), 16);

    ASSERT(digit10->digit == 100)("parsed base 10 wrong");
    ASSERT(digit2->digit == 0b1000)("parsed base 2 wrong");
    ASSERT(digit16->digit == 0xFFA6)("parsed base 16 wrong");

    return report_end("test");
}
