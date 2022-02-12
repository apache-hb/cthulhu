#include "emit.h"

#include "cthulhu/util/str.h"

#include <ctype.h>

static void types_print(FILE *out, vector_t *types) {
    size_t len = vector_len(types);
    if (len == 0) {
        return;
    }
    
    fprintf(out, "types[%zu] {\n", len);
    for (size_t i = 0; i < len; i++) {
        if (i != 0) {
            fprintf(out, ",\n");
        }
        const type_t *type = vector_get(types, i);
        fprintf(out, "  [%zu] = %s", i, type_format(type));
    }
    fprintf(out, "\n}\n");
}

static char *emit_imm(const value_t *imm) {
    if (imm == NULL) {
        return "null";
    }

    const type_t *type = imm->type;
    if (is_literal(type)) {
        return format("untyped(%s)", mpz_get_str(NULL, 10, imm->digit));
    } else if (is_digit(type)) {
        return format("%s", mpz_get_str(NULL, 10, imm->digit));
    } else if (is_bool(type)) {
        return ctu_strdup(imm->boolean ? "true" : "false");
    } else if (is_pointer(type)) {
        return ctu_strdup("nullptr");
    } else {
        return "???";
    }
}

static char *emit_addr(block_t *addr) {
    if (addr == NULL) {
        return ctu_strdup("???");
    }
    
    if (addr->kind == BLOCK_STRING) {
        return format("str[%zu]", addr->idx);
    }

    return format("@%s", addr->name);
}

static char *emit_operand(operand_t op) {
    switch (op.kind) {
    case IMM: return emit_imm(op.imm);
    case LABEL: return format(".%zu", op.label);
    case VREG: return format("%%%zu", op.vreg);
    case ARG: return format("arg(%zu)", op.arg);
    case ADDRESS: return emit_addr(op.block);
    case EMPTY: return ctu_strdup("(empty)");
    default: return ctu_strdup("???");
    }
}

static char *emit_unary(size_t idx, step_t step) {
    const char *op = unary_name(step.unary);
    char *operand = emit_operand(step.operand);
    
    return format("%%%zu = unary %s %s", idx, op, operand);
}

static char *emit_binary(size_t idx, step_t step) {
    const char *op = binary_name(step.binary);
    char *lhs = emit_operand(step.lhs);
    char *rhs = emit_operand(step.rhs);

    return format("%%%zu = binary %s %s %s", idx, op, lhs, rhs);
}

static char *emit_return(step_t step) {
    char *operand = emit_operand(step.operand);
    if (operand == NULL) {
        return ctu_strdup("ret");
    } else {
        return format("ret %s", operand);
    }
}

static char *emit_load(size_t idx, step_t step) {
    char *addr = emit_operand(step.src);
    char *offset = emit_operand(step.offset);
    return format("%%%zu = load %s +%s", idx, addr, offset);
}

static char *emit_jmp(step_t step) {
    char *label = emit_operand(step.label);
    return format("jmp %s", label);
}

static char *emit_branch(step_t step) {
    char *cond = emit_operand(step.cond);
    char *label = emit_operand(step.label);
    char *other = emit_operand(step.other);
    return format("branch %s then %s else %s", cond, label, other);
}

static char *emit_block(size_t idx) {
    return format("\r\r.%zu:", idx);
}

static char *emit_store(step_t step) {
    char *dst = emit_operand(step.dst);
    char *src = emit_operand(step.src);

    return format("store %s %s", dst, src);
}

static char *emit_call(size_t idx, step_t step) {
    char *func = emit_operand(step.func);
    size_t len = oplist_len(step.args);
    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        char *arg = emit_operand(oplist_get(step.args, i));
        vector_set(args, i, arg);
    }

    return format("%%%zu = call %s (%s)", idx, func, strjoin(", ", args));
}

static const char *get_builtin_name(builtin_t builtin) {
    switch (builtin) {
    case BUILTIN_ALIGNOF: return "alignof";
    case BUILTIN_SIZEOF: return "sizeof";
    case BUILTIN_UUIDOF: return "uuidof";
    default: return "???";
    }
}

static char *emit_builtin(size_t idx, step_t step) {
    const char *builtin = get_builtin_name(step.builtin);
    char *target = type_format(step.target);

    return format("%%%zu = builtin %s %s", idx, builtin, target);
}

static char *emit_cast(size_t idx, step_t step) {
    char *src = emit_operand(step.src);
    char *target = type_format(step.type);

    return format("%%%zu = cast %s %s", idx, target, src);
}

static char *emit_offset(size_t idx, step_t step) {
    char *src = emit_operand(step.src);
    char *offset = emit_operand(step.offset);

    return format("%%%zu = offset %s +%s", idx, src, offset);
}

static char *emit_step(block_t *flow, size_t idx) {
    step_t step = flow->steps[idx];
    switch (step.opcode) {
    case OP_EMPTY: return NULL;
    case OP_BINARY: return emit_binary(idx, step);
    case OP_UNARY: return emit_unary(idx, step);
    case OP_RETURN: return emit_return(step);
    case OP_LOAD: return emit_load(idx, step);
    case OP_STORE: return emit_store(step);
    case OP_CALL: return emit_call(idx, step);

    case OP_BUILTIN: return emit_builtin(idx, step);

    case OP_JMP: return emit_jmp(step);
    case OP_BRANCH: return emit_branch(step);
    case OP_BLOCK: return emit_block(idx);

    case OP_CAST: return emit_cast(idx, step);
    case OP_OFFSET: return emit_offset(idx, step);

    default: return format("error %d", step.opcode);
    }
}

static char *get_block_type(block_t *block) {
    return type_format(block->type);
}

static char *emit_names(const char *set, vector_t *locals) {
    if (locals == NULL || vector_len(locals) == 0) {
        return NULL;
    }

    vector_t *all = VECTOR_MAP(locals, get_block_type);
    for (size_t i = 0; i < vector_len(all); i++) {
        char *type = vector_get(all, i);
        block_t *it = vector_get(locals, i);
        const char *name = it->name;
        char *fmt = format("[%s] = %s", name, type);
        vector_set(all, i, fmt);
    }

    return format("%s = { %s }", set, strjoin(", ", all));
}

static void var_print(FILE *out, module_t *mod, size_t idx) {
    block_t *flow = vector_get(mod->vars, idx);
    const char *name = flow->name;
    
    char *locals = emit_names("locals", flow->locals);
    if (locals != NULL) {
        fprintf(out, "  %s\n", locals);
    }

    const char *type = type_format(flow->type);

    if (flow->value != NULL) {
        fprintf(out, "  %s: %s = %s\n", name,
            type,
            value_format(flow->value)  
        );
        return;
    } else {
        fprintf(out, "  %s: %s = %p {\n", name, type, flow->value);
    }

    size_t len = flow->len;

    for (size_t i = 0; i < len; i++) {
        char *step = emit_step(flow, i);
        if (step != NULL) {
            fprintf(out, "    %s\n", step);
        }
    }

    fprintf(out, "  }\n");
}

static void func_print(FILE *out, module_t *mod, size_t idx) {
    block_t *flow = vector_get(mod->funcs, idx);
    if (flow == NULL) { return; }
    const char *name = flow->name;
    
    char *locals = emit_names("locals", flow->locals);
    if (locals != NULL) {
        fprintf(out, "%s\n", locals);
    }

    size_t len = flow->len;
    fprintf(out, "define %s: %s {\n", name, 
        type_format(flow->type)
    );

    for (size_t i = 0; i < len; i++) {
        char *step = emit_step(flow, i);
        if (step != NULL) {
            fprintf(out, "  %s\n", step);
        }
    }

    fprintf(out, "}\n");
}

static void values_print(FILE *out, module_t *mod, vector_t *vars) {
    size_t nvars = vector_len(vars);
    
    if (nvars == 0) {
        return;
    }

    fprintf(out, "values[%zu] {\n", nvars);
    
    for (size_t i = 0; i < nvars; i++) {
        var_print(out, mod, i);
    }

    fprintf(out, "}\n");
}

static void strtab_print(FILE *out, vector_t *strtab) {
    size_t len = vector_len(strtab);
    
    if (len == 0) {
        return;
    }

    fprintf(out, "strtab[%zu] {\n", len);
    for (size_t i = 0; i < len; i++) {
        block_t *block = vector_get(strtab, i);
        fprintf(out, "  %zu: `%s`\n", i, strnorm(block->string));
    }
    fprintf(out, "}\n");
}

static void imports_print(FILE *out, vector_t *imports) {
    size_t len = vector_len(imports);
    
    if (len == 0) {
        return;
    }
    
    fprintf(out, "imports[%zu] {\n", len);
    for (size_t i = 0; i < len; i++) {
        block_t *imp = vector_get(imports, i);
        fprintf(out, "  %s: %s\n", imp->name, type_format(imp->type));
    }
    fprintf(out, "}\n");
}

void module_print(FILE *out, module_t *mod) {
    size_t nfuncs = vector_len(mod->funcs);

    fprintf(out, "module = %s\n", mod->name);
    types_print(out, mod->types);
    imports_print(out, mod->imports);
    strtab_print(out, mod->strtab);
    values_print(out, mod, mod->vars);

    if (nfuncs > 0) {
        fprintf(out, "\n");
    }

    for (size_t i = 0; i < nfuncs; i++) {
        if (i > 0) {
            fprintf(out, "\n");
        }

        func_print(out, mod, i);
    }
}