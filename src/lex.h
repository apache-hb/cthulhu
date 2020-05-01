typedef struct {
    uint64_t pos;
    uint64_t line;
    uint64_t col;
} FilePos;

typedef struct {
    FilePos pos;
    FILE* file;
    int index;
    char buffer[1024];
    char lookahead;
} Lexer;

typedef enum {
    KeywordNone,
    KeywordImport,
    KeywordType,
    KeywordDef,
    KeywordLet,
    KeywordReturn,
    
    KeywordEnum,
    KeywordUnion,
    KeywordVariant,

    KeywordLSquare,
    KeywordRSquare,

    KeywordLParen,
    KeywordRParen,

    KeywordLBrace,
    KeywordRBrace,

    KeywordAssign,
    KeywordBuiltin,
    KeywordArrow,
    KeywordBigArrow,
    KeywordColon,
    KeywordComma,
    KeywordDot,
    KeywordQuestion,

    KeywordNot,
    KeywordBitNot,

    KeywordNEq,
    KeywordEq,

    KeywordSub,
    KeywordSubEq,

    KeywordAdd,
    KeywordAddEq,

    KeywordMul,
    KeywordMulEq
} Keyword;

typedef enum {
    TokenTypeIdent = 0,
    TokenTypeKeyword = 1,
    TokenTypeString = 2,
    TokenTypeChar = 3,
    TokenTypeInt = 4,
    TokenTypeFloat = 5,
    TokenTypeEOF = 6,
    TokenTypeInvalid = 7
} TokenType;

typedef union {
    char* string;
    char* ident;
    char letter;
    double number;
    int64_t integer;
    Keyword keyword;
} TokenData;

typedef struct {
    FilePos pos;
    TokenType type;
    TokenData data;
} Token;

static int IsValidToken(Token tok)
{
    return tok.type != TokenTypeInvalid;
}


static char FileNext(Lexer* lex)
{
    char c = lex->lookahead;
    lex->lookahead = (char)fgetc(lex->file);

    lex->pos.pos += 1;
    if(c == '\n')
    {
        lex->pos.line += 1;
        lex->pos.col = 0;
    }
    else
    {
        lex->pos.col += 1;
    }

    return c;
}

static char FilePeek(Lexer* lex)
{
    return lex->lookahead;
}

static int FileConsume(Lexer* lex, int c)
{
    if(FilePeek(lex) == c)
    {
        FileNext(lex);
        return 1;
    }

    return 0;
}

static char FileSkipWhitespace(Lexer* lex)
{
    char c = FileNext(lex);

    while(isspace(c))
        c = FileNext(lex);

    return c;
}

static char FileSkipComment(Lexer* lex, char c)
{
    while(c != '\n')
        c = FileNext(lex);

    c = FileSkipWhitespace(lex);

    return c;
}

#define NEW_TOKEN(name, typ, field, val) static Token name(FilePos pos, typ data) { \
    Token tok; \
    tok.data.field = data; \
    tok.type = val; \
    tok.pos = pos; \
    return tok; \
}

NEW_TOKEN(NewKeyword, Keyword, keyword, TokenTypeKeyword)
NEW_TOKEN(NewIdent, char*, ident, TokenTypeIdent)
NEW_TOKEN(NewInt, int64_t, integer, TokenTypeInt)

static Token InvalidToken()
{
    Token tok;
    tok.type = TokenTypeInvalid;
    return tok;
}

static Token NewEOF(FilePos pos)
{
    Token tok;
    tok.type = TokenTypeEOF;
    tok.pos = pos;
    return tok;
}

static void BufferAdd(Lexer* lex, char c)
{
    lex->buffer[lex->index++] = c;
    lex->buffer[lex->index] = '\0';
}

static void BufferReset(Lexer* lex)
{
    lex->buffer[0] = '\0';
    lex->index = 0;
}

static Token KeyOrIdent(FilePos pos, Lexer* lex)
{
    if(strcmp(lex->buffer, "def") == 0)
        return NewKeyword(pos, KeywordDef);
    else if(strcmp(lex->buffer, "type") == 0)
        return NewKeyword(pos, KeywordType);
    else if(strcmp(lex->buffer, "import") == 0)
        return NewKeyword(pos, KeywordImport);
    else if(strcmp(lex->buffer, "enum") == 0)
        return NewKeyword(pos, KeywordEnum);
    else if(strcmp(lex->buffer, "variant") == 0)
        return NewKeyword(pos, KeywordVariant);
    else if(strcmp(lex->buffer, "union") == 0)
        return NewKeyword(pos, KeywordUnion);
    else if(strcmp(lex->buffer, "let") == 0)
        return NewKeyword(pos, KeywordLet);
    else if(strcmp(lex->buffer, "return") == 0)
        return NewKeyword(pos, KeywordReturn);
    else
        return NewIdent(pos, strdup(lex->buffer));
}

static Token Symbol(FilePos pos, Lexer* lex, char c)
{
    switch(c)
    {
    case '[':
        return NewKeyword(pos, KeywordLSquare);
    case ']':
        return NewKeyword(pos, KeywordRSquare);
    case '(':
        return NewKeyword(pos, KeywordLParen);
    case ')':
        return NewKeyword(pos, KeywordRParen);
    case '{':
        return NewKeyword(pos, KeywordLBrace);
    case '}':
        return NewKeyword(pos, KeywordRBrace);
    case '@':
        return NewKeyword(pos, KeywordBuiltin);
    case ':':
        return NewKeyword(pos, FileConsume(lex, '=') ? KeywordAssign : KeywordColon);
    case '-':
        if(FileConsume(lex, '>'))
        {
            return NewKeyword(pos, KeywordArrow);
        }
        else if(FileConsume(lex, '='))
        {
            return NewKeyword(pos, KeywordSubEq);
        }
        else
        {
            return NewKeyword(pos, KeywordSub);
        }
    case '=':
        if(FileConsume(lex, '='))
        {
            return NewKeyword(pos, KeywordEq);
        }
        else if(FileConsume(lex, '>'))
        {
            return NewKeyword(pos, KeywordBigArrow);
        }
        else
        {
            return NewKeyword(pos, KeywordNone);
        }
    case '.':
        return NewKeyword(pos, KeywordDot);
    case '~':
        return NewKeyword(pos, KeywordBitNot);
    case '!':
        return NewKeyword(pos, FileConsume(lex, '=') ? KeywordNEq : KeywordNot);
    case '+':
        return NewKeyword(pos, FileConsume(lex, '=') ? KeywordAddEq : KeywordAdd);
    case ',':
        return NewKeyword(pos, KeywordComma);
    case '*':
        return NewKeyword(pos, FileConsume(lex, '=') ? KeywordMulEq : KeywordMul);
    case '?':
        return NewKeyword(pos, KeywordQuestion);
    default:
        printf("%c %x is not a valid keyword {%lu:%lu}\n", c, c, pos.col, pos.line);
        return NewKeyword(pos, KeywordNone);
    }
}

static Token LexerNext(Lexer* lex)
{
    FilePos here;
    char c;

    BufferReset(lex);

    here = lex->pos;
    c = FileSkipWhitespace(lex);

    while(c == '#')
        c = FileSkipComment(lex, c);

    if(c == EOF)
        return NewEOF(here);

    if(isalpha(c) || c == '_')
    {
        BufferAdd(lex, c);
        for(;;)
        {
            c = FilePeek(lex);
            if(isalnum(c) || c == '_')
            {
                BufferAdd(lex, FileNext(lex));
            }
            else
            {
                break;
            }
        }

        return KeyOrIdent(here, lex);
    }
    else if(c == '0')
    {
        c = FilePeek(lex);
        if(c == 'x')
        {
            FileNext(lex);
            for(;;)
            {
                c = FilePeek(lex);
                if(isxdigit(c))
                {
                    BufferAdd(lex, FileNext(lex));
                }
                else
                {
                    break;
                }
            }

            return NewInt(here, strtol(lex->buffer, NULL, 16));
        }
        else if(c == 'b')
        {
            FileNext(lex);
            for(;;)
            {
                c = FilePeek(lex);
                if(c == '1' || c == '0')
                {
                    BufferAdd(lex, FileNext(lex));
                }
                else
                {
                    break;
                }
            }

            return NewInt(here, strtol(lex->buffer, NULL, 2));
        }
        else if(!isdigit(FilePeek(lex)) && FilePeek(lex) != '.')
        {
            return NewInt(here, 0);
        }
        else
        {
            printf("oh no\n");
            exit(400);
        }
    }
    else if(isdigit(c))
    {
        BufferAdd(lex, c);
        for(;;)
        {
            c = FilePeek(lex);
            if(isdigit(c))
            {
                BufferAdd(lex, FileNext(lex));
            }
            else
            {
                break;
            }
        }

        return NewInt(here, strtol(lex->buffer, NULL, 10));
    }
    else if(c == '"')
    {
        exit(400);
    }
    else if(c == '\'')
    {
        exit(400);
    }
    else if(c == '\0')
    {
        return NewEOF(here);
    }
    else
    {
        return Symbol(here, lex, c);
    }
}

static Lexer NewLexer(FILE* file)
{
    Lexer lex;
    
    lex.file = file;
    lex.pos.pos = 0;
    lex.pos.line = 0;
    lex.pos.col = 0;
    lex.lookahead = ' ';
    lex.index = 0;
    lex.buffer[0] = '\0';

    return lex;
}

static void PrintToken(Token tok, FILE* out)
{
    switch(tok.type)
    {
    case TokenTypeKeyword:
        fprintf(out, "Token(Keyword(%d))\n", tok.data.keyword);
        break;
    case TokenTypeString:
        fprintf(out, "Token(String(%s))\n", tok.data.string);
        break;
    case TokenTypeIdent:
        fprintf(out, "Token(Ident(%s))\n", tok.data.ident);
        break;
    case TokenTypeChar:
        fprintf(out, "Token(Char(%c))\n", tok.data.letter);
        break;
    case TokenTypeInt:
        fprintf(out, "Token(Int(%ld))\n", tok.data.integer);
        break;
    case TokenTypeFloat:
        fprintf(out, "Token(Float(%f))\n", tok.data.number);
        break;
    case TokenTypeEOF:
        fprintf(out, "Token(EOF)\n");
        break;
    case TokenTypeInvalid:
        fprintf(out, "Invalid\n");
        break;
    }
}

#if 0
static void TokenFree(Token tok)
{
    if(tok.type == TokenTypeIdent)
        free(tok.data.ident);
    else if(tok.type == TokenTypeString)
        free(tok.data.string);
}
#endif