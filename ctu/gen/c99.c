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

static void plain_include(FILE *out, const char *name) {
    fprintf(out, "#include <%s>\n", name);
}

static void line(FILE *out) {
    fprintf(out, "\n");
}

static const char *gen_type(type_t *type, const char *name);

static char *gen_callable(type_t *type, const char *name) {
    const char *ret = gen_type(type->result, NULL);
    char *body = format("(*%s)", name ? name : "");
    
    size_t nargs = list_len(type->args);
    
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

static const char *get_max(bool sign) {
    return sign ? "intmax_t" : "uintmax_t";
}

static const char *get_ptr(bool sign) {
    return sign ? "intptr_t" : "uintptr_t";
}

static const char *int_name(type_t *type) {
    const char *out;
    switch (type->integer) {
    case INTEGER_CHAR: out = "char"; break;
    case INTEGER_SHORT: out = "short"; break;
    case INTEGER_INT: out = "int"; break;
    case INTEGER_LONG: out = "long long"; break;
    case INTEGER_SIZE: return get_size(is_signed(type));
    case INTEGER_INTMAX: return get_max(is_signed(type));
    case INTEGER_INTPTR: return get_ptr(is_signed(type));
    default:
        assert("int_name unreachable");
        return "";
    }

    if (!is_signed(type)) {
        out = format("unsigned %s", out);
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

static const char *record_type(type_t *type) {
    switch (type->kind) {
    case TYPE_UNION: return "union";
    case TYPE_STRUCT: return "struct";

    default:
        assert("record-type unreachable");
        return "error";
    }
}

static char *gen_record(type_t *type, const char *name) {
    if (name) {
        return format("%s type_%zu_t %s", record_type(type), type->index, name);
    } else {
        return format("%s type_%zu_t", record_type(type), type->index);
    }
}

static const char *gen_string(const char *name) {
    if (name) {
        return format("const char* %s", name);
    } else {
        return "const char*";
    }
}

static const char *gen_array(type_t *type, const char *name) {
    const char *body = gen_type(type->of, NULL);

    if (name) {
        return format("%s %s[%zu]", body, name, type->size);
    } else {
        return format("%s[%zu]", body, type->size);
    }
}

static const char *gen_type(type_t *type, const char *name) {
    const char *ty = NULL;
    
    switch (type->kind) {
    case TYPE_INTEGER: 
        ty = gen_int(type, name); 
        break;
    case TYPE_BOOLEAN: 
        ty = gen_bool(name);
        break;
    case TYPE_VOID: 
        ty = gen_void(name); 
        break;
    case TYPE_CALLABLE: 
        ty = gen_callable(type, name);
        break;
    case TYPE_POINTER: 
        ty = gen_pointer(type, name); 
        break;
    case TYPE_STRING: 
        ty = gen_string(name);
        break;
    case TYPE_STRUCT: case TYPE_UNION:
        ty = gen_record(type, name);
        break;
    case TYPE_ARRAY: 
        ty = gen_array(type, name); 
        break;

    case TYPE_SIZEOF:
        return gen_type(type->of, name);

    default:
        reportf(LEVEL_INTERNAL, nodeof(type), "gen-type(`%s` <- `%s`)", typefmt(type), name);
        return "void";
    }

    if (type->kind != TYPE_VOID && is_const(type)) {
        ty = format("const %s", ty);
    }

    return ty;
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
            const char *name = omit_names ? NULL : genarg(i);
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
    case IMM_BOOL: return imm.b ? "true" : "false";
    case IMM_INT: return format("%s", mpz_get_str(NULL, 0, imm.num));
    case IMM_SIZE: return format("%s", mpz_get_str(NULL, 0, imm.num));

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

static char *genstr(size_t idx) {
    return format("str%zu", idx);
}

static const char *gen_operand_inner(flow_t *flow, operand_t op) {
    switch (op.kind) {
    case BLOCK: return format("block%zu", op.label);
    case VREG: return local(op.vreg);
    case IMM: return gen_imm(op.imm);
    case FUNC: return gen_func(flow->mod, op.func);
    case VAR: return gen_var(op.var);
    case ARG: return genarg(op.arg);
    case STRING: return genstr(op.var);

    case NONE:
        assert("gen_operand invalid operand");
        return "";

    default:
        assert("gen_operand unreachable");
        return "";
    }
}

static const char *gen_operand_indirect(flow_t *flow, type_t *ty, operand_t op, bool indirect) {
    const char *inner = gen_operand_inner(flow, op);

    size_t offset = op.offset;
    const char *off = NULL;
    
    if (offset != SIZE_MAX) {
        if (ty->interop) {
            off = format("%s->%s", inner, ty->fields.fields[offset].name);
        } else {
            off = format("%s->_%zu", inner, offset);
        }
    } else {
        if (indirect) {
            off = format("*%s", inner);
        } else {
            off = format("%s", inner);
        }
    }

    return off;
}

static const char *gen_operand(flow_t *flow, type_t *ty, operand_t op) {
    return gen_operand_indirect(flow, ty, op, false);
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

    case BINARY_AND: return "&&";
    case BINARY_OR: return "||";
    case BINARY_XOR: return "^";

    case BINARY_SHL: return "<<";
    case BINARY_SHR: return ">>";
    case BINARY_BITAND: return "&";
    case BINARY_BITOR: return "|";

    default:
        assert("unreachable get_binary");
        return "";
    }
}

static char *gen_args(flow_t *flow, operand_t *args, size_t len) {
    const char **types = ctu_malloc(sizeof(char*) * len);
    for (size_t i = 0; i < len; i++) {
        types[i] = gen_operand_inner(flow, args[i]);
    }
    return str_join(", ", types, len);
}

static void gen_store(FILE *out, flow_t *flow, step_t *step) {
    const char *src = gen_operand(flow, step->type, step->src);
    const char *dst = gen_operand_indirect(flow, step->type, step->dst, true);

    fprintf(out, "%s = %s;\n", dst, src);
}

static void gen_load(FILE *out, flow_t *flow, size_t idx, step_t *step) {
    const char *src = gen_operand_indirect(flow, step->type, step->src, true);

    fprintf(out, "%s = %s;\n", gen_type(step->type, local(idx)), src);
}

static void gen_builtin(FILE *out, size_t idx, step_t *step) {
    switch (step->builtin) {
    case BUILTIN_SIZEOF:
        fprintf(out, "%s = sizeof(%s);\n", 
            gen_type(size_int(), local(idx)),
            gen_type(step->type, NULL)
        );
        break;
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
        fprintf(out, "goto %s;\n", gen_operand(flow, step->type, step->block)); 
        break;
    case OP_VALUE: 
        fprintf(out, "%s = %s;\n", gen_type(step->type, local(idx)), gen_operand(flow, step->type, step->value)); 
        break;
    case OP_RETURN:
        if (is_void(step->type)) {
            fprintf(out, "return;\n");
        } else {
            fprintf(out, "return %s;\n", gen_operand(flow, step->type, step->value));
        } 
        break;
    case OP_RESERVE:
        fprintf(out, "%s[%zu];\n", gen_type(step->type, local(idx)), step->size);
        break;
    case OP_STORE:
        gen_store(out, flow, step);
        break;
    case OP_LOAD:
        gen_load(out, flow, idx, step);
        break;
    case OP_UNARY:
        fprintf(out, "%s = %s;\n", 
            gen_type(step->type, local(idx)), 
            get_unary(step->unary, gen_operand(flow, step->type, step->expr))
        );
        break;
    case OP_BINARY:
        fprintf(out, "%s = %s %s %s;\n", gen_type(step->type, local(idx)), 
            gen_operand(flow, step->type, step->lhs), 
            get_binary(step->binary), 
            gen_operand(flow, step->type, step->rhs)
        );
        break;

    case OP_CALL:
        if (!is_void(step->type)) {
            fprintf(out, "%s = ", gen_type(step->type, local(idx)));
        }
        fprintf(out, "%s(%s);\n", 
            gen_operand(flow, step->type, step->value), 
            gen_args(flow, step->args, step->len)
        );
        break;

    case OP_BRANCH:
        fprintf(out, "if (%s) { goto %s; } else { goto %s; }\n",
            gen_operand(flow, step->type, step->cond),
            gen_operand(flow, step->type, step->block),
            gen_operand(flow, step->type, step->other)
        );
        break;

    case OP_CONVERT:
        fprintf(out, "%s = (%s)%s;\n", 
            gen_type(step->type, local(idx)),
            gen_type(step->type, NULL),
            gen_operand(flow, step->type, step->value)
        );
        break;

    case OP_OFFSET:
        fprintf(out, "%s = %s + %s;\n",
            gen_type(step->type, local(idx)),
            gen_operand(flow, step->type, step->src),
            gen_operand(flow, step->type, step->index)
        );
        break;

    case OP_BUILTIN:
        gen_builtin(out, idx, step);
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
    ASSERT(is_record(type))("can only emit record types");

    fprintf(out, "%s type_%zu_t { ", record_type(type), type->index);

    fields_t fields = type->fields;
    for (size_t i = 0; i < fields.size; i++) {
        field_t field = fields.fields[i];
        fprintf(out, "%s;", gen_type(field.type, format("_%zu", i)));
    }
    fprintf(out, "};");
}

static void emit_string(FILE *out, size_t idx, const char *str) {
    fprintf(out, "const char *str%zu = ", idx);
    fprintf(out, "\"");
    for (size_t i = 0; i < strlen(str); i++) {
        fprintf(out, "\\x%x", str[i]);
    }
    fprintf(out, "\";");
}

void gen_c99(FILE *out, module_t *mod) {
    char *name = str_replace(mod->name, "/", "_");
    guard_head(out, name);

    line(out);

    add_include(out, "stdbool");
    add_include(out, "stdint");
    add_include(out, "stddef");

    line(out);

    for (size_t i = 0; i < list_len(mod->headers); i++) {
        plain_include(out, list_at(mod->headers, i));
    }

    for (size_t i = 0; i < num_strings(mod); i++) {
        if (i != 0) {
            line(out);
        }
        emit_string(out, i, mod->strings[i]);
    }

    for (size_t i = 0; i < num_types(mod); i++) {
        if (i != 0) {
            line(out);
        }
        type_t *type = mod->types[i];
        if (type->interop) {
            continue;
        }

        if (type->kind == TYPE_SIZEOF)
            type = type->of;

        fprintf(out, "%s;", gen_type(set_mut(type, true), NULL));
    }

    line(out);

    for (size_t i = 0; i < num_types(mod); i++) {
        if (i != 0) {
            line(out);
        }
        type_t *type = mod->types[i];
        if (type->interop) {
            continue;
        }

        if (type->kind == TYPE_SIZEOF)
            type = type->of;
        
        emit_type(out, type);
    }

    line(out);

    for (size_t i = 0; i < num_vars(mod); i++) {
        var_t *var = mod->vars + i;
        if (var->interop) {
            continue;
        }
        
        if (SHOULD_EMIT(var))
            gen_global(out, var, i);
    }

    line(out);

    for (size_t i = 0; i < num_flows(mod); i++) {
        flow_t *flow = mod->flows + i;
        if (flow->interop) {
            continue;
        }

        if (SHOULD_EMIT(flow))
            def_flow(out, mod->flows + i);
    }

    line(out);

    for (size_t i = 0; i < num_flows(mod); i++) {
        flow_t *flow = mod->flows + i;
        if (flow->interop) {
            continue;
        }
        
        if (SHOULD_EMIT(flow) && !flow->stub)
            gen_flow(out, mod->flows + i);
    }

    guard_tail(out, name);
}
