#include "cthulhu.h"

#include <ctype.h>
#include <string.h>

#if !defined(CT_MALLOC) || !defined(CT_REALLOC) || !defined(CT_FREE)
#   error "CT_MALLOC, CT_REALLOC, and CT_FREE must be defined"
#endif

static int isident1(int c) { return isalpha(c) || c == '_'; }
static int isident2(int c) { return isalnum(c) || c == '_'; }

static CtBuffer bufferNew(size_t size)
{
    CtBuffer self;
    self.ptr = CT_MALLOC(size);
    self.len = 0;
    self.alloc = size;

    return self;
}

static void bufferPush(CtBuffer *self, int c)
{
    if (self->len + 1 >= self->alloc)
    {
        self->alloc *= 2;
        self->ptr = CT_REALLOC(self->ptr, self->alloc);
    }

    self->ptr[self->len++] = (char)c;
    self->ptr[self->len] = 0;
}

static int lexNext(CtState *self)
{
    int c = self->ahead;
    self->ahead = self->next(self->stream);

    bufferPush(&self->source, c);

    self->pos.dist++;
    self->len++;

    if (c == '\n')
    {
        self->pos.col = 0;
        self->pos.line++;
    }
    else
    {
        self->pos.col++;
    }

    return c;
}

static int lexPeek(CtState *self)
{
    return self->ahead;
}

static int lexConsume(CtState *self, int c)
{
    if (lexPeek(self) == c)
    {
        lexNext(self);
        return 1;
    }
    return 0;
}

static int lexSkip(CtState *self)
{
    int c = lexNext(self);

    while (1)
    {
        if (c == '#')
        {
            while (lexPeek(self) != '\n')
                lexNext(self);

            c = lexSkip(self);
        }
        else if (isspace(c))
        {
            c = lexNext(self);
        }
        else
        {
            break;
        }
    }

    return c;
}

static size_t lexOff(CtState *self)
{
    return self->pos.dist;
}

struct CtKeyEntry { const char *str; int key; int flags; };

static struct CtKeyEntry keys[] = {
#define KEY(id, str, flags) { str, id, flags },
#include "keys.inc"
    { "", K_INVALID, 0 }
};

#define NUM_KEYS (sizeof(keys) / sizeof(struct CtKeyEntry))

static const char *lexView(CtState *self, size_t off)
{
    return self->source.ptr + off;
}

static void report(CtState *self, CtError *err)
{
    if (self->err_idx < self->max_errs)
        self->errs[self->err_idx++] = *err;

    err->type = ERR_NONE;
}

static void lexIdent(CtState *self, CtToken *tok)
{
    size_t off = lexOff(self) - 1;
    while (isident2(lexPeek(self)))
        lexNext(self);

    for (size_t i = 0; i < NUM_KEYS; i++)
    {
        if (
            /**
             * comparing an extra byte is safe because of the null terminator
             * it also makes sure stuff like `a` doesnt match `alias`
             * (the first letter of both is `a`)
             */
            memcmp(lexView(self, off), keys[i].str, self->len + 1) == 0
            && keys[i].flags & self->flags
        )
        {
            tok->type = TK_KEY;
            tok->data.key = keys[i].key;
            return;
        }
    }

    tok->type = TK_IDENT;
    tok->data.ident.len = self->len;
    tok->data.ident.offset = off;
}

static void lexSymbol(CtState *self, CtToken *tok, int c)
{
    tok->type = TK_KEY;

    switch (c)
    {
    case '!':
        if (lexConsume(self, '<'))
        {
            self->ldepth++;
            tok->data.key = K_TBEGIN;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_NEQ : K_NOT;
        }
        break;

    case '>':
        if (self->ldepth)
        {
            self->ldepth--;
            tok->data.key = K_TEND;
        }
        else if (lexConsume(self, '>'))
        {
            tok->data.key = lexConsume(self, '=') ? K_SHREQ : K_SHR;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_LTE : K_LT;
        }
        break;
    case '<':
        if (lexConsume(self, '<'))
        {
            tok->data.key = lexConsume(self, '=') ? K_SHLEQ : K_SHL;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_GTE : K_GT;
        }
        break;
    case '*':
        tok->data.key = lexConsume(self, '=') ? K_MULEQ : K_MUL;
        break;
    case '/':
        tok->data.key = lexConsume(self, '=') ? K_DIVEQ : K_DIV;
        break;
    case '%':
        tok->data.key = lexConsume(self, '=') ? K_MODEQ : K_MOD;
        break;
    case '^':
        tok->data.key = lexConsume(self, '=') ? K_XOREQ : K_XOR;
        break;
    case '&':
        if (lexConsume(self, '&'))
        {
            tok->data.key = K_AND;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_BITANDEQ : K_BITAND;
        }
        break;
    case '|':
        if (lexConsume(self, '|'))
        {
            tok->data.key = K_OR;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_BITOREQ : K_BITOR;
        }
        break;
    case '@':
        tok->data.key = K_AT;
        break;
    case '?':
        tok->data.key = K_QUESTION;
        break;
    case '~':
        tok->data.key = K_BITNOT;
        break;
    case '(':
        tok->data.key = K_LPAREN;
        break;
    case ')':
        tok->data.key = K_RPAREN;
        break;
    case '[':
        tok->data.key = K_LSQUARE;
        break;
    case ']':
        tok->data.key = K_RSQUARE;
        break;
    case '{':
        tok->data.key = K_LBRACE;
        break;
    case '}':
        tok->data.key = K_RBRACE;
        break;

    case '=':
        if (lexConsume(self, '>'))
        {
            tok->data.key = K_ARROW;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_EQ : K_ASSIGN;
        }
        break;

    case ':':
        tok->data.key = lexConsume(self, ':') ? K_COLON2 : K_COLON;
        break;
    case '+':
        tok->data.key = lexConsume(self, '=') ? K_ADDEQ : K_ADD;
        break;
    case '-':
        if (lexConsume(self, '>'))
        {
            tok->data.key = K_PTR;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_SUBEQ : K_SUB;
        }
        break;
    case '.':
        tok->data.key = K_DOT;
        break;
    case ',':
        tok->data.key = K_COMMA;
        break;
    case ';':
        tok->data.key = K_SEMI;
        break;

    default:
        tok->data.key = K_INVALID;
        self->lerr.type = ERR_INVALID_SYMBOL;
        break;
    }
}

#define LEX_COLLECT(limit, c, pattern, ...) { \
    int i = 0; \
    while (1) { \
        int c = lexPeek(self); \
        if (pattern) { \
            __VA_ARGS__ \
            lexNext(self); \
            if (i++ > limit) {\
                self->lerr.type = ERR_OVERFLOW; \
            }} else { break; }}}

static void lexBase2(CtState *self, CtToken *tok)
{
    size_t out = 0;

    LEX_COLLECT(64, c, (c == '0' || c == '1'), {
        out = (out * 2) + (c == '1');
    })

    tok->data.digit.enc = BASE2;
    tok->data.digit.num = out;
}

#define BASE10_LIMIT (18446744073709551615ULL % 10)
#define BASE10_CUTOFF (18446744073709551615ULL / 10)

static void lexBase10(CtState *self, CtToken *tok, int c)
{
    size_t out = c - '0';

    while (1)
    {
        c = lexPeek(self);
        if (isdigit(c))
        {
            size_t n = c - '0';
            if (out > BASE10_CUTOFF || (out == BASE10_CUTOFF && n > BASE10_LIMIT))
                self->lerr.type = ERR_OVERFLOW;

            out = (out * 10) + n;
        }
        else
        {
            break;
        }

        lexNext(self);
    }

    tok->data.digit.enc = BASE10;
    tok->data.digit.num = out;
}

static void lexBase16(CtState *self, CtToken *tok)
{
    size_t out = 0;
    LEX_COLLECT(16, c, isxdigit(c), {
        uint8_t n = (uint8_t)c;
        size_t v = ((n & 0xF) + (n >> 6)) | ((n >> 3) & 0x8);
        out = (out << 4) | v;
    })

    tok->data.digit.enc = BASE16;
    tok->data.digit.num = out;
}

static void lexDigit(CtState *self, CtToken *tok, int c)
{
    tok->type = TK_INT;

    if (c == '0' && lexConsume(self, 'b'))
    {
        lexBase2(self, tok);
    }
    else if (c == '0' && lexConsume(self, 'x'))
    {
        lexBase16(self, tok);
    }
    else
    {
        lexBase10(self, tok, c);
    }

    if (isident1(lexPeek(self)))
    {
        size_t len = 1;
        size_t off = lexOff(self);
        while (isident2(lexPeek(self)))
        {
            lexNext(self);
            len++;
        }

        tok->data.digit.suffix.offset = off;
        tok->data.digit.suffix.len = len;
    }
    else
    {
        tok->data.digit.suffix.offset = 0;
    }
}

typedef enum {
    /* keep going */
    CR_OK,
    /* found a newline */
    CR_NL,

    /* end of the string */
    CR_END,

    /* end of the file (this is bad) */
    CR_EOF
} CharResult;

static CharResult lexSingleChar(CtState *self, int *out)
{
    int c = lexNext(self);
    *out = c;

    if (c == '\\')
    {
        c = lexNext(self);
        switch (c)
        {
        case 'b': *out = '\b'; break;
        case 'a': *out = '\a'; break;
        case 'f': *out = '\f'; break;
        case 'r': *out = '\r'; break;
        case 'n': *out = '\n'; break;
        case 'v': *out = '\v'; break;
        case 't': *out = '\t'; break;
        case '\'':*out = '\''; break;
        case '"': *out = '\"'; break;
        case '\\':*out = '\\'; break;
        case '0': *out = '\0'; break;
        default:
            *out = c;
            self->lerr.type = ERR_INVALID_ESCAPE;
            break;
        }
    }
    else if (c == '\n')
    {
        return CR_NL;
    }
    else if (c == '"')
    {
        return CR_END;
    }
    else if (c == -1)
    {
        self->lerr.type = ERR_STRING_EOF;
        return CR_EOF;
    }


    return CR_OK;
}

static void lexMultiString(CtState *self, CtToken *tok)
{
    tok->type = TK_STRING;
    int c;
    size_t len = 0;
    size_t off = self->strings.len;
    while (1)
    {
        CharResult res = lexSingleChar(self, &c);
        if (res == CR_END || res == CR_EOF)
        {
            break;
        }

        bufferPush(&self->strings, c);
        len++;
    }

    bufferPush(&self->strings, '\0');

    tok->data.str.len = len;
    tok->data.str.offset = off;
    tok->data.str.multiline = 1;
}

static void lexSingleString(CtState *self, CtToken *tok)
{
    tok->type = TK_STRING;
    int c;
    size_t len = 0;
    size_t off = self->strings.len;
    while (1)
    {
        CharResult res = lexSingleChar(self, &c);
        if (res == CR_NL)
        {
            self->lerr.type = ERR_STRING_LINEBREAK;
        }
        else if (res == CR_END || res == CR_EOF)
        {
            break;
        }

        bufferPush(&self->strings, c);
        len++;
    }

    bufferPush(&self->strings, '\0');

    tok->data.str.len = len;
    tok->data.str.offset = off;
    tok->data.str.multiline = 0;
}

static void lexChar(CtState *self, CtToken *tok)
{
    tok->type = TK_CHAR;
    int c;
    switch (lexSingleChar(self, &c))
    {
    case CR_END:
        c = '"';
        break;

        /* handle eof */
    case CR_EOF:
        tok->data.letter = '\0';
        return;
    case CR_NL:
        tok->data.letter = '\0';
        self->lerr.type = ERR_STRING_LINEBREAK;
        return;
    default:
        break;
    }

    tok->data.letter = c;

    /* check for the trailing ' */
    c = lexNext(self);
    if (c != '\'')
    {
        /* try and gracefully cleanup the users mess */
        size_t len = self->len;
        while (!isspace(lexPeek(self)))
            lexNext(self);
        self->len = len;

        self->lerr.type = ERR_CHAR_CLOSING;
    }
}

static CtToken lexToken(CtState *self)
{
    int c = lexSkip(self);
    self->len = 1;
    CtOffset start = self->pos;

    CtToken tok;

    if (c == -1)
    {
        tok.type = TK_END;
    }
    else if (c == 'r' || c == 'R')
    {
        if (lexConsume(self, '"'))
            lexMultiString(self, &tok);
        else
            lexIdent(self, &tok);
    }
    else if (isident1(c))
    {
        lexIdent(self, &tok);
    }
    else if (c == '"')
    {
        lexSingleString(self, &tok);
    }
    else if (c == '\'')
    {
        lexChar(self, &tok);
    }
    else if (isdigit(c))
    {
        lexDigit(self, &tok, c);
    }
    else
    {
        lexSymbol(self, &tok, c);
    }

    tok.len = self->len;
    tok.pos = start;

    if (self->lerr.type != ERR_NONE)
    {
        tok.type = TK_INVALID;
        self->lerr.len = self->len;
        self->lerr.pos = start;
        report(self, &self->lerr);
    }

    return tok;
}

static CtToken pNext(CtState *self)
{
    CtToken tok = self->tok;

    while (tok.type == TK_INVALID)
    {
        tok = lexToken(self);
    }

    self->tok.type = TK_INVALID;

    return tok;
}

static CtToken pPeek(CtState *self)
{
    self->tok = pNext(self);

    return self->tok;
}

static int pExpect(CtState *self, CtKey key)
{
    CtToken tok = pNext(self);
    if (tok.type != TK_KEY || tok.data.key != key)
    {
        self->perr.type = ERR_UNEXPECTED_KEY;
        self->perr.tok = tok;
        return 0;
    }
    return 1;
}

static int pConsume(CtState *self, CtKey key)
{
    CtToken tok = pNext(self);

    if (tok.type == TK_KEY && tok.data.key == key)
    {
        return 1;
    }

    self->tok = tok;
    return 0;
}

static CtASTArray nodes(size_t size)
{
    CtASTArray arr;
    arr.nodes = CT_MALLOC(sizeof(CtAST) * size);
    arr.len = 0;
    arr.alloc = size;
    return arr;
}

static void addNode(CtASTArray *self, CtAST *node)
{
    if (self->alloc <= self->len + 1)
    {
        self->alloc += 4;
        self->nodes = CT_REALLOC(self->nodes, sizeof(CtAST) * self->alloc);
    }

    self->nodes[self->len++] = *node;
}

static CtASTArray pCollect(CtState *self, CtAST*(*func)(CtState*), CtKey sep, CtKey end)
{
    CtASTArray out = nodes(4);
    do {
        addNode(&out, func(self));
    } while (pConsume(self, sep));

    if (end != K_INVALID)
        pExpect(self, end);

    return out;
}

static CtASTArray pUntil(CtState *self, CtAST*(*func)(CtState*), CtKey sep, CtKey end)
{
    CtASTArray out = nodes(4);
    if (!pConsume(self, end))
    {
        do addNode(&out, func(self));
        while (pConsume(self, sep));
        pExpect(self, end);
    }

    return out;
}

static CtAST *ast(CtASTKind type)
{
    CtAST *out = CT_MALLOC(sizeof(CtAST));
    out->type = type;
    return out;
}

static CtAST *astTok(CtASTKind type, CtToken tok)
{
    CtAST *out = CT_MALLOC(sizeof(CtAST));
    out->type = type;
    out->tok = tok;
    return out;
}

static CtAST *pIdent(CtState *self)
{
    CtToken tok = pNext(self);
    if (tok.type != TK_IDENT)
    {
        self->perr.type = ERR_UNEXPECTED_TOK;
        self->perr.tok = tok;
        return NULL;
    }

    return astTok(AK_IDENT, tok);
}

typedef enum {
    OP_ERROR = 0,

    OP_ASSIGN = 1,
    OP_TERNARY = 2,
    OP_LOGIC = 3,
    OP_EQUAL = 4,
    OP_COMPARE = 5,
    OP_BITS = 6,
    OP_SHIFT = 7,
    OP_MATH = 8,
    OP_MUL = 9
} OpPrec;

static OpPrec prec(CtToken tok)
{
    if (tok.type != TK_KEY)
        return OP_ERROR;

    switch (tok.data.key)
    {
    case K_ASSIGN: case K_ADDEQ: case K_SUBEQ:
    case K_MULEQ: case K_DIVEQ: case K_MODEQ:
    case K_XOREQ: case K_BITANDEQ: case K_BITOREQ:
    case K_SHLEQ: case K_SHREQ:
        return OP_ASSIGN;
    case K_QUESTION:
        return OP_TERNARY;
    case K_AND: case K_OR:
        return OP_LOGIC;
    case K_EQ: case K_NEQ:
        return OP_EQUAL;
    case K_GT: case K_GTE: case K_LT: case K_LTE:
        return OP_COMPARE;
    case K_XOR: case K_BITAND: case K_BITOR:
        return OP_BITS;
    case K_SHL: case K_SHR:
        return OP_SHIFT;
    case K_ADD: case K_SUB:
        return OP_MATH;
    case K_MUL: case K_DIV: case K_MOD:
        return OP_MUL;
    default:
        return OP_ERROR;
    }
}

#define IS_UNARY(key) (key == K_ADD || key == K_SUB || key == K_BITNOT || key == K_NOT || key == K_BITAND || key == K_MUL)

static CtAST *pExpr(CtState *self);
static CtAST *pQuals(CtState *self);
static CtAST *pType(CtState *self);
static CtAST *pStmts(CtState *self);
static CtAST *pStmt(CtState *self);

static CtAST *pArg(CtState *self)
{
    CtAST *arg = ast(AK_ARG);
    if (pConsume(self, K_LSQUARE))
    {
        arg->data.arg.field = pConsume(self, K_ELSE) ? self->empty : pExpr(self);
        pExpect(self, K_RSQUARE);
        pExpect(self, K_ASSIGN);
    }
    else
    {
        arg->data.arg.field = NULL;
    }

    arg->data.arg.expr = pExpr(self);

    return arg;
}

static CtAST *pInit(CtState *self)
{
    CtAST *init = ast(AK_INIT);
    init->data.args = pCollect(self, pArg, K_COMMA, K_RBRACE);
    return init;
}

static CtAST *pArgDecl(CtState *self, int *defaulted)
{
    CtAST *arg = ast(AK_ARGDECL);
    arg->data.argdecl.name = pIdent(self);
    arg->data.argdecl.type = pConsume(self, K_COLON) ? pType(self) : NULL;
    arg->data.argdecl.init = pConsume(self, K_ASSIGN) ? pExpr(self) : NULL;

    *defaulted = arg->data.argdecl.init != NULL;
    return arg;
}

static CtAST *pCapture(CtState *self)
{
    CtAST *node = ast(AK_CAPTURE);
    node->data.capture.ref = pConsume(self, K_BITAND);
    node->data.capture.symbol = pQuals(self);
    return node;
}

static CtAST *pFunc(CtState *self)
{
    CtAST *func = ast(AK_FUNC);
    CtToken tok = pPeek(self);

    if (tok.type == TK_IDENT)
    {
        func->data.func.name = astTok(AK_IDENT, pNext(self));
    }
    else
    {
        func->data.func.name = NULL;
    }

    if (self->pdepth == 0 && !func->data.func.name)
    {
        self->perr.type = ERR_MISSING_NAME;
    }

    if (pConsume(self, K_LPAREN))
    {
        func->data.func.args = nodes(4);
        int defaulted = 0;
        int next_defaulted = 0;
        if (!pConsume(self, K_RPAREN))
        {
            do {
                CtAST *arg = pArgDecl(self, &defaulted);
                addNode(&func->data.func.args, arg);

                /* make sure defaulted args dont have positional args after them */
                if (defaulted)
                    next_defaulted = 1;

                if (next_defaulted && !defaulted)
                    self->perr.type = ERR_DEFAULT_ARG;
            } while (pConsume(self, K_COMMA));
            pExpect(self, K_RPAREN);
        }
    }
    else
    {
        func->data.func.args.len = 0;
    }

    if (pConsume(self, K_COLON))
    {
        pExpect(self, K_LSQUARE);
        func->data.func.captures = pUntil(self, pCapture, K_COMMA, K_RSQUARE);
    }
    else
    {
        func->data.func.captures.len = 0;
    }

    func->data.func.result = pConsume(self, K_PTR) ? pType(self) : NULL;

    if (pConsume(self, K_ARROW))
    {
        func->data.func.body = pExpr(self);
    }
    else if (pConsume(self, K_LBRACE))
    {
        func->data.func.body = pStmts(self);
    }
    else
    {
        func->data.func.body = NULL;
    }

    return func;
}

static CtAST *pBuiltin(CtState *self)
{
    CtAST *name = pQuals(self);
    CtASTArray args;

    /* TODO: how to parse arguments to give to custom parser callback */
    if (pConsume(self, K_LPAREN))
    {
        args = pUntil(self, pArg, K_COMMA, K_RPAREN);
    }
    else
    {
        args.len = 0;
    }

    void *body;

    if (pConsume(self, K_LBRACE))
    {
        self->pbuiltin = 1;
        /* TODO: custom parser callback */

        body = NULL;

        self->pbuiltin = 0;
    }
    else
    {
        body = NULL;
    }

    CtAST *builtin = ast(AK_BUILTIN);
    builtin->data.builtin.name = name;
    builtin->data.builtin.args = args;
    builtin->data.builtin.body = body;

    return builtin;
}

static CtAST *pPrimary(CtState *self)
{
    CtToken tok = pPeek(self);
    CtAST *node = NULL;

    if (tok.type == TK_CHAR || tok.type == TK_INT || tok.type == TK_STRING)
    {
        node = ast(AK_LITERAL);
        node->tok = pNext(self);
    }
    if (tok.type == TK_KEY)
    {
        if (IS_UNARY(tok.data.key))
        {
            node = ast(AK_UNARY);
            node->tok = pNext(self);
            node->data.expr = pPrimary(self);
        }
        else if (tok.data.key == K_LPAREN)
        {
            pNext(self);
            node = pExpr(self);
            if(!pExpect(self, K_RPAREN))
                self->perr.type = ERR_MISSING_BRACE;
        }
        else if (tok.data.key == K_LBRACE)
        {
            pNext(self);
            node = pInit(self);
        }
        else if (tok.data.key == K_DEF)
        {
            pNext(self);
            node = pFunc(self);
        }
        else if (tok.data.key == K_AT)
        {
            /* compiler builtin */
            if (self->pbuiltin)
                self->perr.type = ERR_NESTED_BUILTIN;

            pNext(self);
            node = pBuiltin(self);
        }
    }
    else if (tok.type == TK_IDENT)
    {
        self->tok = tok;
        node = ast(AK_NAME);
        node->data.name.name = pType(self);
        node->data.name.init = pConsume(self, K_LBRACE) ? pInit(self) : NULL;
    }

    if (!node)
        return NULL;

    while (1)
    {
        if (pConsume(self, K_LPAREN))
        {
            CtAST *call = ast(AK_CALL);
            call->data.call.expr = node;
            call->data.call.args = pUntil(self, pArg, K_COMMA, K_RPAREN);
            node = call;
        }
        else if (pConsume(self, K_LSQUARE))
        {
            CtAST *sub = ast(AK_SUB);
            sub->data.sub.expr = node;
            sub->data.sub.index = pExpr(self);
            pExpect(self, K_RSQUARE);
            node = sub;
        }
        else if (pConsume(self, K_DOT))
        {
            CtAST *access = ast(AK_ACCESS);
            access->data.access.expr = node;
            access->data.access.field = pIdent(self);
            node = access;
        }
        else if (pConsume(self, K_PTR))
        {
            CtAST *deref = ast(AK_DEREF);
            deref->data.deref.expr = node;
            deref->data.deref.field = pIdent(self);
            node = deref;
        }
        else
        {
            break;
        }
    }

    return node;
}

static CtAST *binop(CtAST *lhs, CtAST *rhs, CtToken tok)
{
    CtAST *node = ast(AK_BINARY);
    node->tok = tok;
    node->data.binary.lhs = lhs;
    node->data.binary.rhs = rhs;
    return node;
}

static int assoc(OpPrec prec)
{
    return prec != OP_ASSIGN;
}

static CtAST *pBinary(CtState *self, OpPrec mprec)
{
    CtAST *lhs = pPrimary(self);

    while (1)
    {
        CtToken cur = pPeek(self);
        if (!prec(cur) || (prec(cur) < mprec))
            break;

        CtToken op = pNext(self);
        OpPrec nprec = prec(op);

        CtAST *rhs;

        if (nprec == OP_TERNARY)
        {
            rhs = ast(AK_TERNARY);
            rhs->data.ternary.cond = lhs;

            if (pConsume(self, K_COLON))
            {
                rhs->data.ternary.yes = lhs;
                rhs->data.ternary.no = pExpr(self);
            }
            else
            {
                rhs->data.ternary.yes = pExpr(self);
                pExpect(self, K_COLON);
                rhs->data.ternary.no = pExpr(self);
            }

            lhs = rhs;
        }
        else
        {
            rhs = pBinary(self, nprec + assoc(nprec));
            lhs = binop(lhs, rhs, op);
        }
    }

    return lhs;
}

static CtAST *pExpr(CtState *self)
{
    return pBinary(self, OP_ASSIGN);
}


static CtAST *pParam(CtState *self)
{
    CtAST *param = ast(AK_PARAM);

    if (pConsume(self, K_COLON))
    {
        param->data.param.name = pIdent(self);
        pExpect(self, K_ASSIGN);
    }
    else
    {
        param->data.param.name = NULL;
    }

    param->data.param.type = pType(self);

    return param;
}

static CtAST *pQual(CtState *self)
{
    CtAST *qual = ast(AK_QUAL);
    qual->data.qual.name = pIdent(self);
    if (pConsume(self, K_TBEGIN))
    {
        qual->data.qual.params = pCollect(self, pParam, K_COMMA, K_TEND);
    }
    return qual;
}

static CtAST *pQuals(CtState *self)
{
    CtAST *type = ast(AK_QUALS);
    type->data.quals = pCollect(self, pQual, K_COLON2, K_INVALID);
    return type;
}

static CtAST *pType(CtState *self)
{
    CtToken tok = pNext(self);
    CtAST *type = NULL;

    if (tok.type == TK_KEY)
    {
        switch (tok.data.key)
        {
        case K_MUL:
            type = astTok(AK_PTR, tok);
            type->data.ptr = pType(self);
            break;
        case K_LPAREN:
            type = astTok(AK_CLOSURE, tok);
            type->data.closure.args = pUntil(self, pType, K_COMMA, K_RPAREN);
            type->data.closure.result = pConsume(self, K_ARROW) ? pType(self) : NULL;
            break;
        case K_LSQUARE:
            type = astTok(AK_ARRAY, tok);
            type->data.array.type = pType(self);
            type->data.array.size = pConsume(self, K_COLON) ? pExpr(self) : NULL;
            pExpect(self, K_RSQUARE);
            break;
        default:
            break;
        }
    }
    else if (tok.type == TK_IDENT)
    {
        self->tok = tok;
        type = pQuals(self);
    }

    return type;
}

static CtAST *pStmts(CtState *self)
{
    CtAST *node = ast(AK_STMTS);

    node->data.stmts = nodes(4);

    self->pdepth++;

    while (1)
    {
        CtAST *expr = pStmt(self);
        if (!expr)
            break;
        addNode(&node->data.stmts, expr);
    }

    pExpect(self, K_RBRACE);

    self->pdepth--;

    return node;
}

static CtAST *pStmt(CtState *self)
{
    CtAST *node;

    node = pExpr(self);
    if (node)
    {
        pExpect(self, K_SEMI);
    }
    else if (pConsume(self, K_LBRACE))
    {
        node = pStmts(self);
    }
    else if (pConsume(self, K_SEMI))
    {
        /* empty statement */
        node = self->empty;
    }

    return node;
}

CtAST *pImport(CtState *self)
{
    CtAST *node = ast(AK_IMPORT);
    node->data.include.path = pCollect(self, pIdent, K_COLON2, K_INVALID);
    if (pConsume(self, K_LPAREN))
    {
        node->data.include.items = pCollect(self, pIdent, K_COMMA, K_RPAREN);
    }
    else
    {
        node->data.include.items.len = 0;
    }
    pExpect(self, K_SEMI);

    return node;
}

/**
 * public api
 */

void ctStateNew(
    CtState *self,
    CtStateInfo info
)
{
    self->stream = info.stream;
    self->next = info.next;
    self->ahead = info.next(info.stream);
    self->name = info.name;

    self->source = bufferNew(0x1000);
    self->strings = bufferNew(0x1000);

    self->pos.source = self;
    self->pos.dist = 0;
    self->pos.col = 0;
    self->pos.line = 0;

    self->ldepth = 0;
    self->pdepth = 0;
    self->pbuiltin = 0;

    self->flags = LF_DEFAULT;

    self->lerr.type = ERR_NONE;
    self->perr.type = ERR_NONE;

    self->errs = info.errs;
    self->max_errs = info.max_errs;
    self->err_idx = 0;

    self->tok.type = TK_INVALID;
    self->empty = ast(AK_OTHER);
    self->empty->tok.pos.source = self;
}

void ctAddFlag(CtState *self, CtLexerFlag flag)
{
    self->flags |= flag;
}

CtAST *ctParseInterp(CtState *self)
{
    (void)self;
    return NULL;
}

CtAST *ctParse(CtState *self)
{
    CtAST *node = pStmt(self);
    if (self->perr.type != ERR_NONE)
        report(self, &self->perr);

    return node;
}

int ctValidate(CtState *self, CtAST *node)
{
    return 1;
}