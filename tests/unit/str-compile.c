// ast unit tests

#include "ctu/ast/compile.h"
#include "ctu/util/report.h"

int main(void) {
    report_begin(1, true);
    types_init();
    
    node_t *nodes = compile_string("test/main", 
        "def main(): int { return 100; }\n",
        NULL
    );

    free_ast_list(all_decls(nodes), true);

    return report_end("test");
}
