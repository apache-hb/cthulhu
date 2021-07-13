// ast unit tests

#include "ctu/ast/ast.h"
#include "ctu/util/report.h"

static where_t NOWHERE = { 0, 0, 0, 0 };

int main(void) {
    report_begin(1, true);
    
    node_t *digit10 = ast_digit(NULL, NOWHERE, "100", 10);

    get_resolved_name(digit10);

    return report_end("test");
}
