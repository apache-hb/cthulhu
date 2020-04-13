typedef struct {
    uint64_t pos;
    uint64_t line;
    uint64_t col;
} FilePos;

typedef struct {
    FilePos pos;
    FILE* file;
    char lookahead;
    char buffer[1024];
    int index;
} Lexer;

typedef enum {
    KeywordImport,
    KeywordType,
    KeywordDef,

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
    KeywordColon,

    KeywordSub,
    KeywordSubEq
} Keyword;

typedef enum {
    TokenTypeIdent = 0,
    TokenTypeKeyword = 1,
    TokenTypeString = 2,
    TokenTypeChar = 3,
    TokenTypeInt = 4,
    TokenTypeFloat = 5,
    TokenTypeEOF = 6
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

int FileNext(Lexer* lex)
{
    int c = lex->lookahead;
    lex->lookahead = fgetc(lex->file);
    
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

int FilePeek(Lexer* lex)
{
    return lex->lookahead;
}

int FileConsume(Lexer* lex, int c)
{
    if(FilePeek(lex) == c)
    {
        FileNext(lex);
        return 1;
    }

    return 0;
}

int FileSkipWhitespace(Lexer* lex)
{
    int c = FileNext(lex);

    while(isspace(c))
        c = FileNext(lex);

    return c;
}

int FileSkipComment(Lexer* lex)
{
    int c = FileNext(lex);

    while(c != '\n')
        c = FileNext(lex);

    return c;
}

#define NEW_TOKEN(name, typ, field, val) Token name(FilePos pos, typ data) { \
    Token tok; \
    tok.data.field = data; \
    tok.type = val; \
    tok.pos = pos; \
    return tok; \
}

NEW_TOKEN(NewKeyword, Keyword, keyword, TokenTypeKeyword)
NEW_TOKEN(NewIdent, char*, ident, TokenTypeIdent)
NEW_TOKEN(NewInt, int64_t, integer, TokenTypeInt)

Token NewEOF(FilePos pos)
{
    Token tok;
    tok.type = TokenTypeEOF;
    tok.pos = pos;
    return tok;
}

void BufferAdd(Lexer* lex, char c)
{
    lex->buffer[lex->index++] = c;
    lex->buffer[lex->index] = '\0';
}

void BufferReset(Lexer* lex)
{
    lex->buffer[0] = '\0';
    lex->index = 0;
}

Token KeyOrIdent(FilePos pos, Lexer* lex)
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
    else
        return NewIdent(pos, strdup(lex->buffer));
}

Token Symbol(FilePos pos, Lexer* lex, int c)
{
    switch(c)
    {
    case '[':
        return NewKeyword(pos, KeywordLSquare);
    case ']':
        return NewKeyword(pos, KeywordRSquare);
    case '(':
        return NewKeyword(pos, KeywordRParen);
    case ')':
        return NewKeyword(pos, KeywordLParen);
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
    default:
        break;
    }
}

Token LexerNext(Lexer* lex)
{
    BufferReset(lex);

    FilePos here = lex->pos;
    int c = FileSkipWhitespace(lex);

    while(c == '#')
        c = FileSkipComment(lex);

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
        else
        {
            
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

    }
    else if(c == '\'')
    {

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

Lexer NewLexer(FILE* file)
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

void PrintToken(Token tok, FILE* out)
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
    }
}

void TokenFree(Token tok)
{
    if(tok.type == TokenTypeIdent)
        free(tok.data.ident);
    else if(tok.type == TokenTypeString)
        free(tok.data.string);
}
