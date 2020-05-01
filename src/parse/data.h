#define VECTOR_TYPE char*
#define VECTOR_NAME str
#include "vector.h"

#define VECTOR_TYPE void*
#define VECTOR_NAME node
#include "vector.h"

typedef struct { char* key; void* node; } KeyNode;
#define VECTOR_TYPE KeyNode
#define VECTOR_NAME keynode
#include "vector.h"

typedef struct { char* key; void* value; void* type; } KeyValType;
#define VECTOR_TYPE KeyValType
#define VECTOR_NAME keyvaltype
#include "vector.h"

#define REPEAT(parser, end, delim, body) \
    if(!ConsumeKeyword(parser, end)) \
    { do { body; } while(ConsumeKeyword(parser, delim)); ExpectKeyword(parser, end); }


typedef struct {
    Lexer* lex;

    Token tok;
} Parser;

Parser NewParser(Lexer* lex)
{
    Parser ret;

    ret.lex = lex;
    ret.tok = InvalidToken();

    return ret;
}


typedef enum {
    /* toplevel statements */
    NodeTypeProgram,
    NodeTypeImport,
    NodeTypeTypedef,

    /* attributes */
    NodeTypePacked,
    NodeTypeAlign,

    /* types */
    NodeTypeName,
    NodeTypeStruct,
    NodeTypeTuple,
    NodeTypeUnion,
    NodeTypeEnum,
    NodeTypeVariant,
    NodeTypePtr,
    NodeTypeArray,

    NodeTypeUnary,
    NodeTypeBinary,
    NodeTypeIdent,
    NodeTypeInt,
    NodeTypeTupleInit,
    NodeTypeStructInit,
    NodeTypeArrayInit
} NodeType;

typedef struct Node {
    NodeType type;

    union {
        struct { vec_str_struct path; char* alias; } _import;
        struct { char* name; struct Node* type; } _typedef;
        struct { char* name; vec_node_struct imports; vec_node_struct body; } _program;
    
        struct Node* _packed;
        struct Node* _align;

        struct { Keyword op; struct Node* expr; } _unary;
        struct { Keyword op; struct Node* lhs; struct Node* rhs; } _binary;

        vec_node_struct _scope;

        vec_node_struct _tuple;
        vec_node_struct _array;
        vec_keynode_struct _struct;

        char* _ident;
        int64_t _int;

        struct {
            vec_node_struct attribs;

            union {
                vec_str_struct _name;
                vec_keynode_struct _struct;
                vec_node_struct _tuple;
                vec_keynode_struct _union;
                vec_keynode_struct _enum;
                vec_keyvaltype_struct _variant;
                struct Node* _ptr;
                struct { struct Node* type; struct Node* size; } _array;
            };
        } type;
    } data;
} Node;

Node* NewNode(NodeType type)
{
    Node* out = malloc(sizeof(Node));
    out->type = type;
    return out;
}

Token NextToken(Parser* parser)
{
    Token tok;

    if(IsValidToken(parser->tok))
    {
        tok = parser->tok;
        parser->tok = InvalidToken();
    }
    else
    {
        tok = LexerNext(parser->lex);
    }

    return tok;
}

Token NextKeyword(Parser* parser)
{
    Token tok;
    
    tok = NextToken(parser);

    if(tok.type != TokenTypeKeyword)
    {
        printf("expected keyword %d %s {%ld:%ld}\n", tok.type, tok.data.ident, tok.pos.col+1, tok.pos.line);
        exit(500);
    }

    return tok;
}

Token NextIdent(Parser* parser)
{
    Token tok = NextToken(parser);

    if(tok.type != TokenTypeIdent)
    {
        printf("expected ident\n");
        exit(500);
    }
    
    return tok;
}

void ExpectKeyword(Parser* parser, Keyword key)
{
    Token tok = NextKeyword(parser);
    if(tok.data.keyword != key)
    {
        printf("incorrect keyword found ");
        PrintToken(tok, stdout);
        exit(500);
    }
}

int ConsumeKeyword(Parser* parser, Keyword key)
{
    Token tok;
    
    tok = parser->tok;
    
    if(!IsValidToken(parser->tok))
        tok = NextToken(parser);

    if(tok.type == TokenTypeKeyword && tok.data.keyword == key)
    {
        parser->tok = InvalidToken();
        return 1;
    }
    else
    {
        parser->tok = tok;
    }

    return 0;
}

void ExpectIdent(Parser* parser, const char* ident)
{
    Token tok;

    tok = NextIdent(parser);

    if(strcmp(tok.data.ident, ident))
    {
        printf("expecting ident %s\n", ident);
        exit(500);
    }
}

char* ConsumeIdent(Parser* parser)
{
    Token tok;

    tok = parser->tok;
    if(!IsValidToken(parser->tok))
        tok = NextToken(parser);

    if(tok.type == TokenTypeIdent)
    {
        parser->tok = InvalidToken();
        return tok.data.ident;
    }
    else
    {
        parser->tok = tok;
    }

    return NULL;
}

Token ExpectToken(Parser* parser, TokenType type)
{
    Token tok;
    tok = NextToken(parser);

    if(tok.type != type)
    {
        printf("incorrect token type\n");
        exit(500);
    }

    return tok;
}

Token PeekToken(Parser* parser, TokenType type)
{
    Token tok;

    tok = NextToken(parser);

    if(tok.type != type)
    {
        parser->tok = tok;
        tok = InvalidToken();
    }

    return tok;
}

KeyNode MakePair(char* key, Node* val)
{
    KeyNode pair;
    pair.key = key;
    pair.node = val;
    return pair;
}

KeyValType MakeKeyValType(char* key, Node* val, Node* type)
{
    KeyValType kvt;

    kvt.key = key;
    kvt.value = val;
    kvt.type = type;

    return kvt;
}
