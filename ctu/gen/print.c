#include "emit.h"

#include "ctu/util/str.h"

#include <ctype.h>

static char *emit_imm(const value_t *imm) {
    if (imm == NULL) {
        return "???";
    }

    const type_t *type = imm->type;
    if (type->type == TY_INTEGER) {
        return format("%s", mpz_get_str(NULL, 10, imm->digit));
    } else {
        return NULL;
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
    vector_t *args = vector_of(step.len);
    for (size_t i = 0; i < step.len; i++) {
        char *arg = emit_operand(step.args[i]);
        vector_set(args, i, arg);
    }

    return format("%%%zu = call %s (%s)", idx, func, strjoin(", ", args));
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

    case OP_JMP: return emit_jmp(step);
    case OP_BRANCH: return emit_branch(step);
    case OP_BLOCK: return emit_block(idx);

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
    
    char *locals = emit_names("  locals", flow->locals);
    if (locals != NULL) {
        fprintf(out, "  %s\n", locals);
    }

    char *params = emit_names("  params", flow->params);
    if (params != NULL) {
        fprintf(out, "%s\n", params);
    }

    const char *type = type_format(flow->type);

    if (flow->value != NULL) {
        fprintf(out, "  %s: %s = %s {\n", name,
            type,
            value_format(flow->value)  
        );
    } else {
        fprintf(out, "  %s: %s {\n", name, type);
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
    const char *name = flow->name;
    
    char *locals = emit_names("locals", flow->locals);
    if (locals != NULL) {
        fprintf(out, "%s\n", locals);
    }

    char *params = emit_names("params", flow->params);
    if (params != NULL) {
        fprintf(out, "%s\n", params);
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
    
    fprintf(out, "values[%zu] {\n", nvars);
    
    for (size_t i = 0; i < nvars; i++) {
        var_print(out, mod, i);
    }

    fprintf(out, "}\n");
}

static void strtab_print(FILE *out, vector_t *strtab) {
    size_t len = vector_len(strtab);
    fprintf(out, "strtab[%zu] {\n", len);
    for (size_t i = 0; i < len; i++) {
        const char *str = vector_get(strtab, i);
        fprintf(out, "  %zu: `%s`\n", i, strnorm(str));
    }
    fprintf(out, "}\n");
}

static void imports_print(FILE *out, vector_t *imports) {
    size_t len = vector_len(imports);
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
