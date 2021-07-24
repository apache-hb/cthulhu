// ast unit tests

#include "ctu/ast/ast.h"
#include "ctu/util/report.h"

int main(void) {
    report_begin(1, true);
    types_init();
    
    node_t *node = ast_return(NULL, NOWHERE, NULL);

    type_t *type = get_type(node);

    ASSERT(is_unresolved(type))("untagged node had type");

    return report_end("test");
}
