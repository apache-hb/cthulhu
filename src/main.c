#include "ast.h"
#include "bison.h"
#include "flex.h"
#include "sema.h"
#include "emit.h"
#include "scanner.h"

int main(int argc, const char **argv) {
    FILE* source;
    const char *path;

    if (argc > 1) {
        source = fopen(argv[1], "r");
        path = argv[1];
    } else {
        source = stdin;
        path = "stdin";
    }

    modules_t *prog = compile_program(path, source);

    printf("%zu modules\n", prog->length);

    for (size_t i = 0; i < prog->length; i++) {
        module_t *mod = prog->data + i;

        dump_module(mod);
    }

    return 0;
}

