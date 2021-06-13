#include "util/report.h"
#include "util/util.h"
#include "front/front.h"
#include "middle/middle.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "back/llvm/llvm.h"
#include "back/gcc/gcc.h"
#include "back/qbe/qbe.h"

typedef enum {
    LOCAL, LLVM, GCC, QBE, WASMTIME
} backend_t;

typedef enum {
    I8086, X86, X64
} arch_t;

static backend_t backend = LOCAL;
static arch_t arch = X64;
static const char *target = NULL;

#define AUTO_STR "auto"

#define LOCAL_STR "local"
#define LLVM_STR "llvm"
#define GCC_STR "gcc"
#define QBE_STR "qbe"
#define WASMTIME_STR "wasmtime"

static const char *set_backend(const char *str) {
    if (startswith(str, AUTO_STR)) {
        return str + strlen(AUTO_STR);
    }

    if (startswith(str, LOCAL_STR)) {
        backend = LOCAL;
        return str + strlen(LOCAL_STR);
    }

    if (startswith(str, LLVM_STR)) {
        backend = LLVM;
        return str + strlen(LLVM_STR);
    }

    if (startswith(str, GCC_STR)) {
        backend = GCC;
        return str + strlen(GCC_STR);
    }

    if (startswith(str, QBE_STR)) {
        backend = QBE;
        return str + strlen(QBE_STR);
    }

    if (startswith(str, WASMTIME_STR)) {
        backend = WASMTIME;
        return str + strlen(WASMTIME_STR);
    }

    reportf("unable to parse backend from target `%s`", str);
    return str;
}

#define I8086_STR "i8086"
#define X86_STR "x86"
#define X64_STR "x64"

static const char *set_arch(const char *str) {
    if (startswith(str, AUTO_STR)) {
        return str + strlen(AUTO_STR);
    }

    if (startswith(str, I8086_STR)) {
        arch = I8086;
        return str + strlen(I8086_STR);
    }

    if (startswith(str, X86_STR)) {
        arch = X86;
        return str + strlen(X86_STR);
    }

    if (startswith(str, X64_STR)) {
        arch = X64;
        return str + strlen(X64_STR);
    }

    reportf("unable to parse arch from target `%s`", target);
    return str;
}

static void set_target(const char *str) {
    target = str;
    str = set_backend(str) + 1;
    str = set_arch(str) + 1;
}

static const char *name = NULL;
static const char *path = NULL;
static const char *expr = NULL;
static const char *output = "a.out";

static bool print_ast = false;
static bool print_ir = false;
static bool print_backend = false;

static const char *status(bool enabled) {
    return enabled ? "installed" : "not installed";
}

static void list_quads(void) {
    puts("quad listings:");

    printf("\tlocal: %s\n", status(true));
    puts("\t\tarch: i8086, x86, x64");
    puts("\t\tformat: elf, coff");
    puts("\t\ttype: exec, static, shared");
    
    bool has_llvm = llvm_enabled();
    printf("\tllvm: %s\n", status(has_llvm));
    if (has_llvm) {
        puts("\t\tarch: x86, x64");
        puts("\t\tformat: elf, coff");
        puts("\t\ttype: exec, static, shared");
    }

    bool has_gcc = gcc_enabled();
    printf("\tgcc: %s\n", status(has_gcc));
    if (has_gcc) {
        puts("\t\tarch: i8086, x86, x64");
        puts("\t\tformat: elf, coff");
        puts("\t\ttype: exec, static, shared");
    }

    bool has_qbe = qbe_enabled();
    printf("\tqbe: %s\n", status(has_qbe));
    if (has_qbe) {
        puts("\t\tarch: x64");
        puts("\t\tformat: elf");
        puts("\t\ttype: exec, static, shared");
    }

    /*
    bool has_cranelift = cranelift_enabled();
    printf("\tcranelift: %s\n", status(has_cranelift));
    if (has_cranelift) {
        puts("\t\tarch: wasm");
        puts("\t\tformat: elf");
        puts("\t\ttype: exec, static, shared");
    }*/
    exit(0);
}

static void print_help(void) {
    printf("%s: cthulhu compiler\n", name);
    puts("\t--help: display help message");
    puts("\t--print-ast: print text representation of ast (default false)");
    puts("\t--print-ir: print text representation of ir (default false)");
    puts("\t--print-back: print text representation of backend (default false)");
    puts("\t--expr <expr>: compile an expression");
    puts("\t--output <path>: change output path (default a.out)");
    puts("\t--target <quad>: specify a target quad `backend-arch-format-type` (default local-native-native-exec)");
    puts("\t--quads: list all available quads");
    exit(0);
}

#define HAS_NEXT (index < argc - 1)
#define GET_NEXT (argv[index + 1])

static const char *get_next(
    const char *arg, int index, 
    int argc, char **argv,
    int *step
) {
    const char *current = argv[index];
    size_t arglen = strlen(arg);
    size_t curlen = strlen(current);

    if (curlen > arglen) {
        current += arglen + 1;
        return current[0] == '=' ? current + 1 : current;
    } else if (HAS_NEXT) {
        *step += 1;
        return GET_NEXT;
    } else {
        reportf("argument `%s` requires a parameter", arg);
        return NULL;
    }
}

#define EXPR "--expr"
#define TARGET "--target"
#define OUTPUT "--output"

static int parse_arg(int index, int argc, char **argv) {
    const char *current = argv[index];
    int step = 1;

    if (startswith(current, "--help")) {
        print_help();
    } else if (startswith(current, "--print-ast")) {
        print_ast = true;
    } else if (startswith(current, "--print-ir")) {
        print_ir = true;
    } else if (startswith(current, "--print-backend")) {
        print_backend = true;
    } else if (startswith(current, OUTPUT)) {
        output = get_next(OUTPUT, index, argc, argv, &step);
    } else if (startswith(current, EXPR)) {
        expr = get_next(EXPR, index, argc, argv, &step);
    } else if (startswith(current, TARGET)) {
        const char *next = get_next(TARGET, index, argc, argv, &step);
        if (next) {
            set_target(next);
            check_errors("target parsing");
        }
    } else if (startswith(current, "--quads")) {
        list_quads();
    } else if (!startswith(current, "-")) {
        // TODO: there has to be a better way of checking if something 
        // is a file we want to parse
        path = current;
    } else {
        fprintf(stderr, "unknown arg `%s`\n", current);
    }

    return step;
}

static nodes_t *compile_input(void) {
    if (expr) {
        return compile_string("expr", expr);
    }

    FILE *file = fopen(path, "r");

    if (!file) {
        reportf("failed to open %s", path);
        return NULL;
    }

    return compile_file(path, file);
}

static void llvm_output_unit(unit_t *unit) {
    llvm_context *ctx = llvm_compile(unit);
    check_errors("llvm compilation");

    FILE *file = fopen(output, "w");
    if (!file) {
        reportf("failed to open output `%s`", output);
        return;
    }

    llvm_output(ctx, file);
    check_errors("llvm output");
}

static void output_unit(unit_t *unit) {
    switch (backend) {
    case LLVM: llvm_output_unit(unit); break;

    default:
        reportf("TODO: %d\n", backend);
        break;
    }
}

static int compile_main(void) {
    set_debug(stdout);

    nodes_t *nodes = compile_input();
    check_errors("frontend");
    
    if (print_ast) {
        for (size_t i = 0; i < nodes->len; i++) {
            debug_ast(nodes->data + i);
            printf("\n");
        }
    }

    unit_t unit = transform_ast(nodes);
    check_errors("middle");

    if (print_ir) {
        debug_unit(&unit);
    }

    output_unit(&unit);
    check_errors("backend");

    return 0;
}

int main(int argc, char **argv) {
    name = argv[0];

#if CTU_FUZZING
    (void)parse_arg;
    if (1 > argc) {
        report("incorrect argc count for fuzzing");
        return 0;
    }
        
    program = argv[1];
    print_ast = true;
    print_ir = true;
#else
    if (argc == 1) {
        print_help();
    }
    
    int i = 1;
    while (i < argc) {
        i += parse_arg(i, argc, argv);
    }

    if (!path && !expr) {
        print_help();
    }

    check_errors("command line");
#endif

    int res = compile_main();
    check_errors("compiler");
    return res;
}
