// ast unit tests

#include "ctu/ast/compile.h"
#include "ctu/util/report.h"

int main(void) {
    init_pool();
    report_begin(1, true);
    
    nodes_t *nodes = compile_string("test/main", 
        "def main(): int { return 100; }\n",
        NULL
    );

    free_ast_list(nodes, true);

    return report_end("test");
}
