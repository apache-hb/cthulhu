#include "emit.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static FILE *s, *h;
static bool emitting_header = 0;

static void
sfmt(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(emitting_header ? h : s, fmt, args);
    va_end(args);
}

static void 
emit_stmt(node_t *stmt);

static void 
emit_type(node_t *type)
{
    switch (type->kind) {
    case NODE_BUILTIN_TYPE:
        if (!type->mut) {
            sfmt("const ");
        }

        sfmt("%s", type->decl.builtin.cname);
        
        break;
    case NODE_POINTER:
        emit_type(type->type);
        sfmt("*");
        break;
    case NODE_RECORD:
        sfmt("struct %s", type->decl.name);
        break;
    case NODE_TYPENAME:
        fprintf(stderr, "typename(%s) leaked past sema\n", type->name);
        break;
    default:
        fprintf(stderr, "unknown type %d\n", type->kind);
        break;
    }
}

static void 
emit_str(char *text)
{
    while (*text) {
        char c = *text++;
        switch (c) {
        case '\n':
            sfmt("\\n");
            break;
        case '\r':
            break;
        default:
            sfmt("%c", c);
            break;
        }
    }
}

static void
emit_call_or_record_init(node_t *stmt)
{
    dump_node(stmt->call.body);
    printf("\n");

    size_t i;

    switch (stmt->call.body->kind) {
    case NODE_RECORD: case NODE_NAME:
        sfmt("{");
        for (i = 0; i < stmt->call.args->length; i++) {
            if (i) {
                sfmt(", ");
            }
            emit_stmt(stmt->call.args->data + i);
        }
        sfmt("}");
        break;
    case NODE_FUNC:
        sfmt("%s(", stmt->call.body->decl.name);
        for (i = 0; i < stmt->call.args->length; i++) {
            if (i) {
                sfmt(", ");
            }
            emit_stmt(stmt->call.args->data + i);
        }
        sfmt(")");
        break;
    default:
        fprintf(stderr, "not call or record %d\n", stmt->call.body->kind);
    }
}

static void 
emit_stmt(node_t *stmt)
{
    size_t i = 0;
    node_t *node;
    switch (stmt->kind) {
    case NODE_COMPOUND:
        sfmt("{");
        for (; i < stmt->compound->length; i++) {
            node = stmt->compound->data + i;
            emit_stmt(node);
            if (node->kind != NODE_COMPOUND && node->kind != NODE_BRANCH)
                sfmt(";");
        }
        sfmt("}");
        break;
    case NODE_VAR:
        if (emitting_header && stmt->exported) {
            sfmt("extern ");
        }
        emit_type(stmt->decl.var.type);
        sfmt(" %s", stmt->decl.name);
        if (stmt->decl.var.init && !emitting_header) {
            sfmt(" = ");
            emit_stmt(stmt->decl.var.init);
        }
        break;
    case NODE_BINARY:
        sfmt("(");
        emit_stmt(stmt->binary.lhs);
        switch (stmt->binary.op) {
        case BINARY_ADD: sfmt(" + "); break;
        case BINARY_SUB: sfmt(" - "); break;
        case BINARY_MUL: sfmt(" * "); break;
        case BINARY_DIV: sfmt(" / "); break;
        case BINARY_REM: sfmt(" %% "); break;
        default: printf("unknown binop\n"); break;
        }
        emit_stmt(stmt->binary.rhs);
        sfmt(")");
        break;
    case NODE_UNARY:
        switch (stmt->unary.op) {
        case UNARY_ABS: sfmt("ctu_abs("); emit_stmt(stmt->unary.expr); sfmt(")"); break;
        case UNARY_NEG: sfmt("("); emit_stmt(stmt->unary.expr); sfmt(" * -1)"); break;
        case UNARY_DEREF: sfmt("(*"); emit_stmt(stmt->unary.expr); sfmt(")"); break;
        case UNARY_REF: sfmt("(&"); emit_stmt(stmt->unary.expr); sfmt(")"); break;
        case UNARY_NOT: sfmt("(!"); emit_stmt(stmt->unary.expr); sfmt(")"); break;
        default: sfmt("unknown unop\n"); break;
        }
        break;
    case NODE_TERNARY:
        sfmt("(");
        emit_stmt(stmt->ternary.cond);
        sfmt("?");
        emit_stmt(stmt->ternary.yes);
        sfmt(":");
        emit_stmt(stmt->ternary.no);
        sfmt(")");
        break;
    case NODE_CALL:
        emit_call_or_record_init(stmt);
        break;
    case NODE_RETURN:
        sfmt("return");
        if (stmt->expr) {
            sfmt(" ");
            emit_stmt(stmt->expr);
        }
        break;
    case NODE_DIGIT:
        if (stmt->digit.base == 16) {
            sfmt("0x");
        }
        sfmt("%s", stmt->digit.digit);
        if (stmt->digit.suffix) {
            sfmt("%s", stmt->digit.suffix);
        }
        break;
    case NODE_NAME:
        sfmt("%s", stmt->name);
        break;
    case NODE_ASSIGN:
        emit_stmt(stmt->assign.old);
        sfmt(" = ");
        emit_stmt(stmt->assign.expr);
        break;
    case NODE_BOOL:
        sfmt("%d", stmt->boolean);
        break;
    case NODE_STRING:
        emit_str(stmt->text);
        break;
    case NODE_BRANCH:
        if (stmt->branch.cond) {
            sfmt("if (");
            emit_stmt(stmt->branch.cond);
            sfmt(")");
        }
        emit_stmt(stmt->branch.body);
        if (stmt->branch.next) {
            sfmt("else");
            emit_stmt(stmt->branch.next);
        }
        break;
    case NODE_ACCESS:
        emit_stmt(stmt->access.expr);
        sfmt(".%s", stmt->access.field);
        break;
    case NODE_WHILE:
        sfmt("while (");
        emit_stmt(stmt->loop.cond);
        sfmt(")");
        emit_stmt(stmt->loop.body);
        break;
    case NODE_BREAK:
        sfmt("break");
        break;
    case NODE_CONTINUE:
        sfmt("continue");
        break;
    case NODE_ARG:
        emit_stmt(stmt->arg.expr);
        break;
    case NODE_RECORD:
        break;
    default:
        fprintf(stderr, "unknown node %d\n", stmt->kind);
    }
}

static void 
emit_param(node_t *decl)
{
    emit_type(decl->decl.param);
    sfmt(" %s", decl->decl.name);
}

static void
emit_attrib(node_t *node)
{
    sfmt("%s", node->decl.name);
    if (node->decl.args && node->decl.args->length) {
        sfmt("(");
        for (size_t i = 0; i < node->decl.args->length; i++) {
            if (i) {
                sfmt(", ");
            }
            node_t *arg = node->decl.args->data + i;
            emit_stmt(arg);
        }
        sfmt(")");
    }
}

static void
emit_attribs(nodes_t *nodes)
{
    if (nodes && nodes->length) {
        sfmt("__attribute__((");

        for (size_t i = 0; i < nodes->length; i++) {
            if (i) {
                sfmt(", ");
            }
            node_t *attrib = nodes->data + i;
            emit_attrib(attrib);
        }

        sfmt("))");
    }
}

static void 
emit_func(node_t *func)
{
    emit_attribs(func->decl.attribs);
    emit_type(func->decl.func.result);
    sfmt(" %s(", func->decl.name);
    size_t len = func->decl.func.params->length;

    if (len == 0) {
        sfmt("void");
    } else {
        for (size_t i = 0; i < len; i++) {
            if (i) {
                sfmt(", ");
            }
            emit_param(func->decl.func.params->data + i);
        }
    }
    sfmt(")");

    if (emitting_header) {
        sfmt(";");
        return;
    }

    emit_stmt(func->decl.func.body);
}

static void
emit_record(node_t *node)
{
    sfmt("struct %s {", node->decl.name);
    for (size_t i = 0; i < node->decl.fields->length; i++) {
        node_t *field = (node->decl.fields->data + i);
        emit_type(field->decl.param);
        sfmt(" %s;", field->decl.name);
    }
    sfmt("}");
    emit_attribs(node->decl.attribs);
    sfmt(";");
}

static void
emit_array_var(node_t *decl)
{
    emit_type(decl->decl.var.type->array.type);
    sfmt(" %s[", decl->decl.name);
    emit_stmt(decl->decl.var.type->array.size);
    sfmt("]");
}

static void 
emit_decl(node_t *decl)
{
    switch (decl->kind) {
    case NODE_FUNC: 
        if (!decl->exported) {
            sfmt("static ");
        }
        emit_func(decl);
        break;
    case NODE_VAR:
        if (!decl->exported && !emitting_header) {
            sfmt("static ");
        } else if (!decl->exported && emitting_header) {
            break;
        }
        emit_attribs(decl->decl.attribs);
        if (decl->decl.var.type->kind == NODE_ARRAY) {
            emit_array_var(decl);
        } else {
            emit_stmt(decl);
        }
        sfmt(";");
        break;
    case NODE_RECORD:
        emit_record(decl);
        break;
    default:
        fprintf(stderr, "unknown decl to emit %d\n", decl->kind);
        break;
    }
    sfmt("\n");
}

static bool wants_header = 0;

void 
emit(char *name, FILE *source, FILE *header, node_t *prog)
{
    s = source;

    /* if we have a header then include it */
    char *temp = strrchr(name, '.');
    if (temp != NULL && strcmp(temp, ".h") == 0) {
        wants_header = true;
    }

    if (wants_header) {
        sfmt("#include \"%s\"\n", name);
    }

    sfmt("#include <stdint.h>\n#include <stddef.h>\n");

    for (size_t i = 0; i < prog->compound->length; i++) {
        node_t *decl = prog->compound->data + i;
        if (wants_header && decl->kind == NODE_RECORD) {
            continue;
        }

        emit_decl(decl);
    }

    if (!wants_header)
        return;

    h = header;
    emitting_header = true;

    size_t l = strlen(name);
    for (size_t i = 0; i < l; i++) {
        switch (name[i]) {
        case '.': case '/': 
            name[i] = '_';
            break;
        default:
            break;
        }
    }

    sfmt("#ifndef %s_h\n", name);
    sfmt("#define %s_h\n", name);
    sfmt("#include <stdint.h>\n#include <stddef.h>\n");
    for (size_t i = 0; i < prog->compound->length; i++) {
        emit_decl(prog->compound->data + i);
    }

    sfmt("#endif /* %s_h */\n", name);
}
