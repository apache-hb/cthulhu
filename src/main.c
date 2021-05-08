#include "ast.h"
#include "bison.h"
#include "flex.h"
#include "sema.h"
#include "emit.h"

#include <stdlib.h>
#include <stdio.h>

static bool dump_ast = 0,
    dump_sema = 0,
    dump_ir = 0;

#define OUTPUT_ARG "--output="
static const size_t output_arg_len = strlen(OUTPUT_ARG);

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: provide input file\n", argv[0]);
        return 1;
    }

    FILE* file;
    FILE* output_header = stdout;
    FILE* output_source = stdout;
    char *target = strdup("out");
    
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strcmp(arg, "--dump-ast") == 0) {
            dump_ast = true;
        } else if (strcmp(arg, "--dump-sema") == 0) {
            dump_sema = true;
        } else if (strcmp(arg, "--dump-ir") == 0) {
            dump_ir = true;
        } else if (strncmp(arg, OUTPUT_ARG, output_arg_len) == 0) {
            const char *path = arg + output_arg_len;
            size_t len = strlen(path) + 3;
            target = calloc(1, len);

            strcpy(target, path);
            strcpy(target + len - 3, ".c");
            output_source = fopen(target, "w");
            if (!output_source) {
                fprintf(stderr, "failed to open output file `%s`\n", target);
                return 1;
            }

            strcpy(target, path);
            strcpy(target + len - 3, ".h");
            output_header = fopen(target, "w");
            if (!output_header) {
                fprintf(stderr, "failed to open output file `%s`\n", target);
                return 1;
            }
        } else {
            file = fopen(arg, "r");

            if (!file) {
                fprintf(stderr, "failed to open source file `%s`\n", arg);
                return 1;
            }
        }  
    }

    void *scanner;
    node_t *ast;

    yylex_init(&scanner);
    yyset_in(file, scanner);

    int err = yyparse(scanner, &ast);

    yylex_destroy(scanner);

    if (err) {
        return err;
    }

    if (dump_ast) {
        dump_node(ast);
        printf("\n");
    }

    state_t state;

    state.inttype = new_builtin_type("int", SINT, 4, "int32_t");
    state.voidtype = new_builtin_type("void", UNIT, 0, "void");
    state.booltype = new_builtin_type("bool", BOOL, 0, "_Bool");
    state.chartype = new_builtin_type("char", SINT, 1, "int8_t");

    nodes_t *types = empty_node_list();
    types = node_append(types, state.inttype);
    types = node_append(types, state.voidtype);
    types = node_append(types, state.booltype);
    types = node_append(types, state.chartype);
    types = node_append(types, 
        new_builtin_type("u8", UINT, 1, "uint8_t"));
    types = node_append(types, 
        new_builtin_type("u16", UINT, 2, "uint16_t"));
    types = node_append(types, 
        new_builtin_type("u32", UINT, 4, "uint32_t"));
    types = node_append(types, 
        new_builtin_type("u64", UINT, 8, "uint64_t"));
    types = node_append(types, 
        new_builtin_type("uintptr", UINT, 8, "uintptr_t"));
    types = node_append(types, 
        new_builtin_type("usize", UINT, 8, "size_t"));

    state.decls = types;

    sema(&state, ast);

    if (state.errors > 0) {
        fprintf(stderr, "%d compilation error(s), aborting\n", state.errors);
        return 1;
    }

    if (dump_sema) {
        dump_node(ast);
        printf("\n");
    }

    emit(target, output_source, output_header, ast);

    return err;
}

/* for syntax errors */
int yyerror(YYLTYPE *loc, yyscan_t scan, node_t **node, const char *msg) {
    (void)scan;
    (void)node;
    fprintf(stderr, "error[%d:%d]: %s\n", loc->first_line, loc->first_column, msg);
    return 1;
}
