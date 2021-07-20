#include "c99.h"

#include "ctu/util/str.h"
#include "ctu/util/report.h"
#include "ctu/util/util.h"

#include <stdlib.h>
#include <inttypes.h>

static void guard_head(FILE *out, const char *name) {
    fprintf(out, "#ifndef %s\n", name);
    fprintf(out, "#define %s\n", name);
}

static void guard_tail(FILE *out, const char *name) {
    fprintf(out, "#endif /* %s */\n", name);
}

static void add_include(FILE *out, const char *name) {
    fprintf(out, "#include <%s.h>\n", name);
}

static void line(FILE *out) {
    fprintf(out, "\n");
}

static const char *gen_type(type_t *type, const char *name);

static char *gen_callable(type_t *type, const char *name) {
    const char *ret = gen_type(type->result, NULL);
    char *body = format("(*%s)", name ? name : "");
    
    size_t nargs = type->args->size;
    
    // make a copy of this pointer list so we can store const pointers
    // in it and still be able to free it later
    char **args = ctu_malloc(sizeof(char*) * nargs);

    for (size_t i = 0; i < nargs; i++) {
        /**
         * why are you casting a const char* to a char* you may ask?
         * because i know better than the compiler, thats why.
         */
        args[i] = (char*)gen_type(type->args->data[i], NULL);
    }

    char *out = format("%s%s(%s)", ret, body, str_join(", ", (const char **)args, nargs));
    free(args);

    return out;
}

static const char *get_size(bool sign) {
    return sign ? "ssize_t" : "size_t";
}

static const char *int_name(type_t *type) {
    const char *out;
    switch (type->integer) {
    case INTEGER_CHAR: out = "char"; break;
    case INTEGER_SHORT: out = "short"; break;
    case INTEGER_INT: out = "int"; break;
    case INTEGER_LONG: out = "long long"; break;
    case INTEGER_SIZE: out = get_size(is_signed(type)); break;
    default:
        assert("int_name unreachable");
        return "";
    }

    if (!is_signed(type)) {
        out = format("unsigned %s", out);
    }

    if (is_const(type)) {
        out = format("const %s", out);
    }

    return out;
}

static const char *gen_int(type_t *type, const char *name) {
    const char *i = int_name(type);
    if (name) {
        return format("%s %s", i, name);
    } else {
        return i;
    }
}

static const char *gen_bool(const char *name) {
    if (name) {
        return format("bool %s", name);
    } else {
        return "bool";
    }
}

static const char *gen_void(const char *name) {
    if (name) {
        return format("void %s", name);
    } else {
        return "void";
    }
}

static char *gen_pointer(type_t *type, const char *name) {
    const char *body = gen_type(type->ptr, NULL);

    if (name) {
        return format("%s* %s", body, name);
    } else {
        return format("%s*", body);
    }
}

static char *gen_struct(type_t *type, const char *name) {
    if (name) {
        return format("struct type_%zu_t %s", type->index, name);
    } else {
        return format("struct type_%zu_t", type->index);
    }
}

static const char *gen_type(type_t *type, const char *name) {
    switch (type->kind) {
    case TYPE_INTEGER: return gen_int(type, name);
    case TYPE_BOOLEAN: return gen_bool(name);
    case TYPE_VOID: return gen_void(name);
    case TYPE_CALLABLE: return gen_callable(type, name);
    case TYPE_POINTER: return gen_pointer(type, name);
    case TYPE_STRUCT: return gen_struct(type, name);

    default:
        assert("unreachable branch in gen_type");
        return "void";
    }
}

static char *genarg(size_t idx) {
    char *out = format("arg%zu", idx);
    return out;
}

#define SHOULD_EMIT(it) (it->exported || it->used)

static void gen_func_decl(FILE *out, flow_t *flow, bool omit_names) {
    if (!flow->exported) {
        fprintf(out, "static ");
    }
    fprintf(out, "%s %s(", gen_type(flow->result, NULL), flow->name);
    
    if (flow->nargs == 0) {
        fprintf(out, "void");
    } else {
        for (size_t i = 0; i < flow->nargs; i++) {
            if (i != 0) {
                fprintf(out, ", ");
            }
            arg_t *arg = flow->args + i;
            const char *name = NULL;
            if (!omit_names) {
                name = genarg(i);
            }
            /* omit argument names if function predefines */
            fprintf(out, "%s", gen_type(arg->type, name));
        }
    }
    fprintf(out, ")");
}

static void def_flow(FILE *out, flow_t *flow) {
    gen_func_decl(out, flow, true);
    fprintf(out, ";\n");
}

static const char *gen_imm(imm_t imm) {
    switch (imm.kind) {
    case IMM_BOOL: return imm.imm_bool ? "true" : "false";
    case IMM_INT: return format("%" PRId64, imm.imm_int);
    case IMM_SIZE: return format("%zu", imm.imm_size);

    default:
        assert("unreachable gen_imm");
        return "";
    }
}

static char *local(size_t idx) {
    return format("local%zu", idx);
}

static char *gen_var(size_t idx) {
    return format("global%zu", idx);
}

static const char *gen_func(module_t *mod, size_t idx) {
    return mod->flows[idx].name;
}

static const char *gen_operand(flow_t *flow, operand_t op) {
    switch (op.kind) {
    case BLOCK: return format("block%zu", op.label);
    case VREG: return local(op.vreg);
    case IMM: return gen_imm(op.imm);
    case FUNC: return gen_func(flow->mod, op.func);
    case VAR: return gen_var(op.var);
    case ARG: return genarg(op.arg);

    case NONE:
        assert("gen_operand invalid operand");
        return "";

    default:
        assert("gen_operand unreachable");
        return "";
    }
}

static char *get_unary(unary_t op, const char *it) {
    switch (op) {
    case UNARY_ABS: return format("llabs(%s)", it);
    case UNARY_NEG: return format("-%s", it);

    default:
        assert("unreachable get_unary");
        return format("%s", it);
    }
}

static const char *get_binary(binary_t op) {
    switch (op) {
    case BINARY_ADD: return "+";
    case BINARY_DIV: return "/";
    case BINARY_SUB: return "-";
    case BINARY_MUL: return "*";
    case BINARY_REM: return "%";

    case BINARY_GT: return ">";
    case BINARY_GTE: return ">=";

    case BINARY_LT: return "<";
    case BINARY_LTE: return "<=";

    case BINARY_EQ: return "==";
    case BINARY_NEQ: return "!=";

    default:
        assert("unreachable get_binary");
        return "";
    }
}

static char *gen_args(flow_t *flow, operand_t *args, size_t len) {
    const char **types = ctu_malloc(sizeof(char*) * len);
    for (size_t i = 0; i < len; i++) {
        types[i] = gen_operand(flow, args[i]);
    }
    return str_join(", ", types, len);
}

static void gen_store(FILE *out, flow_t *flow, step_t *step) {
    const char *dst = gen_operand(flow, step->dst);
    const char *src = gen_operand(flow, step->src);

    if (step->dst.offset != SIZE_MAX) {
        /* TODO: at the point we can store a struct in a register fix this */
        fprintf(out, "%s[0]._%zu = %s;\n", dst, step->dst.offset, src);
    } else {
        fprintf(out, "%s[0] = %s;\n", dst, src);
    }
}

static void gen_step(FILE *out, flow_t *flow, size_t idx) {
    step_t *step = flow->steps + idx;

    switch (step->opcode) {
    case OP_BLOCK: 
        /* the (void)0 cast makes clang accept this code */
        fprintf(out, "block%zu:(void)0;\n", idx); 
        break;
    case OP_JUMP: 
        fprintf(out, "goto %s;\n", gen_operand(flow, step->block)); 
        break;
    case OP_VALUE: 
        fprintf(out, "%s = %s;\n", gen_type(step->type, local(idx)), gen_operand(flow, step->value)); 
        break;
    case OP_RETURN:
        if (is_void(step->type)) {
            fprintf(out, "return;\n");
        } else {
            fprintf(out, "return %s;\n", gen_operand(flow, step->value));
        } 
        break;
    case OP_RESERVE:
        fprintf(out, "%s[1];\n", gen_type(step->type, local(idx)));
        break;
    case OP_STORE:
        gen_store(out, flow, step);
        break;
    case OP_LOAD:
        fprintf(out, "%s = *%s;\n", gen_type(step->type, local(idx)), gen_operand(flow, step->src));
        break;
    case OP_UNARY:
        fprintf(out, "%s = %s;\n", gen_type(step->type, local(idx)), get_unary(step->unary, gen_operand(flow, step->expr)));
        break;
    case OP_BINARY:
        fprintf(out, "%s = %s %s %s;\n", gen_type(step->type, local(idx)), 
            gen_operand(flow, step->lhs), 
            get_binary(step->binary), 
            gen_operand(flow, step->rhs)
        );
        break;

    case OP_CALL:
        if (!is_void(step->type)) {
            fprintf(out, "%s = ", gen_type(step->type, local(idx)));
        }
        fprintf(out, "%s(%s);\n", gen_operand(flow, step->value), gen_args(flow, step->args, step->len));
        break;

    case OP_BRANCH:
        fprintf(out, "if (%s) { goto %s; } else { goto %s; }\n",
            gen_operand(flow, step->cond),
            gen_operand(flow, step->block),
            gen_operand(flow, step->other)
        );
        break;

    case OP_CONVERT:
        fprintf(out, "%s = (%s)%s", 
            gen_type(step->type, local(idx)),
            gen_type(step->type, NULL),
            gen_operand(flow, step->value)
        );
        break;

    case OP_EMPTY: break;

    default:
        assert("gen_step unreachable %d", step->opcode);
    }
}

static void gen_flow(FILE *out, flow_t *flow) {
    gen_func_decl(out, flow, false);
    fprintf(out, "\n{\n");

    for (size_t i = 0; i < flow->len; i++) {
        gen_step(out, flow, i);
    }

    fprintf(out, "}\n\n");
}

static void gen_global(FILE *out, var_t *var, size_t idx) {
    fprintf(out, "%s[1];\n", 
        gen_type(var->type, gen_var(idx))
    );
}

static void emit_type(FILE *out, type_t *type) {
    ASSERT(type->kind == TYPE_STRUCT)("can only emit structs");

    fprintf(out, "struct type_%zu_t { ", type->index);

    record_t fields = type->fields;
    for (size_t i = 0; i < fields.size; i++) {
        field_t field = fields.fields[i];
        fprintf(out, "%s;", gen_type(field.type, format("_%zu", i)));
    }
    fprintf(out, "};");
}

void gen_c99(FILE *out, module_t *mod) {
    char *name = str_replace(mod->name, "/", "_");
    guard_head(out, name);

    line(out);

    add_include(out, "stdbool");
    add_include(out, "stdint");
    add_include(out, "stddef");

    for (size_t i = 0; i < num_types(mod); i++) {
        if (i != 0) {
            line(out);
        }
        type_t *type = mod->types[i];
        fprintf(out, "%s;", gen_type(type, NULL));
    }

    line(out);

    for (size_t i = 0; i < num_types(mod); i++) {
        if (i != 0) {
            line(out);
        }
        type_t *type = mod->types[i];
        emit_type(out, type);
    }

    line(out);

    for (size_t i = 0; i < num_vars(mod); i++) {
        var_t *var = mod->vars + i;
        if (SHOULD_EMIT(var))
            gen_global(out, var, i);
    }

    line(out);

    for (size_t i = 0; i < num_flows(mod); i++) {
        flow_t *flow = mod->flows + i;
        if (SHOULD_EMIT(flow))
            def_flow(out, mod->flows + i);
    }

    line(out);

    for (size_t i = 0; i < num_flows(mod); i++) {
        flow_t *flow = mod->flows + i;
        if (SHOULD_EMIT(flow))
            gen_flow(out, mod->flows + i);
    }

    guard_tail(out, name);
}
