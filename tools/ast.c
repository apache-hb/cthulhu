#include <stdlib.h>
#include <stdio.h>

#define CT_MALLOC malloc
#define CT_REALLOC realloc
#define CT_FREE free

#include "cthulhu/cthulhu.c"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

CtState state;

static int next(void *ptr) { return fgetc(ptr); }

static void range(CtView view, CtBuffer src)
{
    for (size_t i = 0; i < view.len; i++)
    {
        int c = src.ptr[view.offset + i];
        if (c == '\n')
            break;
        putc(c, stdout);
    }
}

static void range2(CtOffset off, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        int c = off.source->source.ptr[off.dist + i - 1];
        if (c == '\n')
            break;
        putc(c, stdout);
    }
}

static void ident(CtToken tok)
{
    range(tok.data.ident, tok.pos.source->source);
}

static void key(CtToken tok)
{
    switch (tok.data.key)
    {
#define KEY(id, str, flags) case id: printf(str); break;
#define OP(id, str) case id: printf("%s", str); break;
#include "cthulhu/keys.inc"
    default: printf("INVALID"); break;
    }
}

static void bin(size_t num)
{
    while (num)
    {
        printf("%d", (num & 1) == 1);
        num >>= 1;
    }
}

static void num(CtToken tok)
{
    switch (tok.data.digit.enc)
    {
    case BASE2:
        printf("0b");
        bin(tok.data.digit.num);
        break;
    case BASE10:
        printf("%zu", tok.data.digit.num);
        break;
    case BASE16:
        printf("0x%zx", tok.data.digit.num);
        break;
    }

    if (tok.data.digit.suffix.offset)
        range(tok.data.digit.suffix, tok.pos.source->source);
}

static void letter(CtToken tok)
{
    printf("%zu", tok.data.letter);
}

static void string(CtToken tok)
{
    if (tok.data.str.multiline)
        printf("R(");

    for (size_t i = 0; i < tok.data.str.len; i++)
    {
        char c = tok.pos.source->strings.ptr[tok.data.str.offset + i];
        switch (c)
        {
        case '\0': printf("\\0"); break;
        case '\n': printf("\\n"); break;
        default: putc(c, stdout); break;
        }
    }

    if (tok.data.str.multiline)
        printf(")");
}

static void wtok(CtToken tok)
{
    switch (tok.type)
    {
    case TK_END: printf("eof"); break;
    case TK_INVALID: printf("invalid"); break;
    case TK_IDENT: printf("ident("); ident(tok); printf(")"); break;
    case TK_KEY: printf("key("); key(tok); printf(")"); break;
    case TK_INT: printf("int("); num(tok); printf(")"); break;
    case TK_CHAR: printf("char("); letter(tok); printf(")"); break;
    case TK_STRING: printf("string("); string(tok); printf(")"); break;
    }
}

static void underline(size_t indent, size_t num)
{
    for (size_t i = 0; i < indent; i++)
        putc(' ', stdout);
    for (size_t i = 0; i < num; i++)
        putc('^', stdout);
}

static void ptok(CtToken tok)
{
    size_t begin = tok.pos.dist - tok.pos.col;
    printf("%s [%zu:%zu]: ", tok.pos.source->name, tok.pos.line, tok.pos.col);
    wtok(tok);
    printf("\n | \n");
    printf(" | ");
    size_t len = 0;
    while (1)
    {
        char c = tok.pos.source->source.ptr[begin++];
        if (c == '\n' || c == '\0')
            break;
        putc(c, stdout);
        len++;
    }
    printf("\n | ");
    underline(tok.pos.col - 1, MIN(tok.len, len));
    printf("\n");
}

static void error(CtError err)
{
    printf("error [%d]: ", err.type);
    switch (err.type)
    {
    case ERR_NONE:
        printf("no error\n");
        break;
    case ERR_OVERFLOW:
        printf("integer literal ");
        range2(err.pos, err.len);
        printf(" is too large to fit into largest available integer\n");
        break;
    case ERR_INVALID_SYMBOL:
        range2(err.pos, err.len);
        printf(" is an invalid symbol\n");
        break;
    case ERR_STRING_LINEBREAK:
        printf("single line literal ");
        range2(err.pos, err.len);
        printf(" contains a line break\n");
        break;
    default:
        printf("internal compiler error\n");
        break;
    }
}

static int errors(CtState *self)
{
    int ret = 0;
    for (size_t i = 0; i < self->err_idx; i++)
    {
        ret = 1;
        error(self->errs[i]);
    }

    self->err_idx = 0;

    return ret;
}

static void pliteral(CtAST *node)
{
    switch (node->tok.type)
    {
    case TK_INT: num(node->tok); break;
    case TK_CHAR: letter(node->tok); break;
    case TK_STRING: string(node->tok); break;
    default: printf("FUCK"); break;
    }
}

static void pnode(CtAST *node)
{
    if (!node)
    {
        printf("NULL");
        return;
    }

    switch (node->type)
    {
    case AK_BINARY:
        printf("(");
        pnode(node->data.binary.lhs);
        printf(" ");
        key(node->tok);
        printf(" ");
        pnode(node->data.binary.rhs);
        printf(")");
        break;
    case AK_LITERAL:
        pliteral(node);
        break;
    case AK_UNARY:
        printf("(");
        key(node->tok);
        pnode(node->data.expr);
        printf(")");
        break;
    case AK_TERNARY:
        printf("(");
        pnode(node->data.ternary.cond);
        printf(" ? ");
        pnode(node->data.ternary.yes);
        printf(" : ");
        pnode(node->data.ternary.no);
        printf(")");
        break;
    case AK_IDENT:
        ident(node->tok);
        break;
    case AK_QUALS:
        for (size_t i = 0; i < node->data.quals.len; i++)
        {
            if (i)
                printf("::");
            pnode(&node->data.quals.nodes[i]);
        }
        break;
    case AK_QUAL:
        pnode(node->data.qual.name);
        if (node->data.qual.params.len)
        {
            printf("!<");

            for (size_t i = 0; i < node->data.qual.params.len; i++)
            {
                if (i)
                    printf(", ");
                pnode(&node->data.qual.params.nodes[i]);
            }

            printf(">");
        }
        break;
    case AK_PARAM:
        if (node->data.param.name)
        {
            printf(":");
            pnode(node->data.param.name);
            printf(" = ");
        }
        pnode(node->data.param.type);
        break;
    case AK_STMTS:
        printf("{ ");
        for (size_t i = 0; i < node->data.stmts.len; i++)
        {
            pnode(&node->data.stmts.nodes[i]);
            printf("; ");
        }
        printf("}");
        break;
    case AK_OTHER:
        if (node == state.empty)
            printf(";");
        else
            printf("EMPTY");
        break;
    case AK_INIT:
        printf("{ ");
        for (size_t i = 0; i < node->data.args.len; i++)
        {
            if (i)
                printf(", ");
            pnode(&node->data.args.nodes[i]);
        }
        printf(" }");
        break;
    case AK_ARG:
        if (node->data.arg.field)
        {
            printf("[");
            if (node->data.arg.field == state.empty)
                printf("else");
            else
                pnode(node->data.arg.field);
            printf("] = ");
        }
        pnode(node->data.arg.expr);
        break;
    case AK_FUNC:
        printf("def ");
        if (node->data.func.name)
            pnode(node->data.func.name);
        printf("(");
        for (size_t i = 0; i < node->data.func.args.len; i++)
        {
            if (i)
                printf(", ");
            pnode(&node->data.func.args.nodes[i]);
        }
        printf(")");

        if (node->data.func.captures.len)
        {
            printf(": [");

            for (size_t i = 0; i < node->data.func.captures.len; i++)
            {
                if (i)
                    printf(", ");
                pnode(&node->data.func.captures.nodes[i]);
            }

            printf("]");
        }

        if (node->data.func.result)
        {
            printf(" -> ");
            pnode(node->data.func.result);
        }

        if (node->data.func.body)
        {
            if (node->data.func.body->type == AK_STMTS)
            {
                pnode(node->data.func.body);
            }
            else
            {
                printf("=> ");
                pnode(node->data.func.body);
            }
        }
        break;
    case AK_CAPTURE:
        if (node->data.capture.ref)
            printf("&");
        pnode(node->data.capture.symbol);
        break;
    case AK_ARGDECL:
        pnode(node->data.argdecl.name);
        if (node->data.argdecl.type)
        {
            printf(": ");
            pnode(node->data.argdecl.type);
        }
        if (node->data.argdecl.init)
        {
            printf(" = ");
            pnode(node->data.argdecl.init);
        }
        break;
    case AK_NAME:
        pnode(node->data.name.name);
        if (node->data.name.init)
        {
            printf(" ");
            pnode(node->data.name.init);
        }
        break;
    case AK_CALL:
        printf("(");
        pnode(node->data.call.expr);
        printf(")");

        printf("(");
        for (size_t i = 0; i < node->data.call.args.len; i++)
        {
            if (i)
                printf(", ");
            pnode(&node->data.call.args.nodes[i]);
        }
        printf(")");
        break;
    default:
        printf("ERROR %d", node->type);
        break;
    }
}

int main(void)
{
    CtStateInfo info;

    info.stream = stdin;
    info.next = next;
    info.name = "stdin";

    CtError errs[20];
    info.errs = errs;
    info.max_errs = 20;

    printf(">>> ");
    ctStateNew(&state, info);

    while (1)
    {
        CtAST *node = ctParse(&state);
        ctValidate(&state, node);
        if (!errors(&state))
        {
            if (!node)
                printf("failed to parse statement");
            else
                pnode(node);
            printf("\n>>> ");
        }
    }

    (void)ptok;

    /*for (;;)
    {
        CtToken tok = lexToken(&state);

        if (!errors(&state))
            ptok(tok);

        if (tok.type == TK_END)
            break;
    }*/
}
