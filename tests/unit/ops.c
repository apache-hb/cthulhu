// ast unit tests

#include "ctu/ast/ast.h"
#include "ctu/util/report.h"

int main(void) {
    report_begin(1, true);
    
    ASSERT(is_math_op(BINARY_ADD))("binary add was not math");

    ASSERT(!is_math_op(BINARY_LTE))("binary lte was a math op");

    return report_end("test");
}
