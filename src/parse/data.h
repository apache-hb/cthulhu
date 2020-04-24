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

typedef enum {
    
    /*
    // programDecl
    //  : importDecl* programBody*
    //  ;
    //
    // programBody
    //  : typeDef 
    //  | funcDef
    //  | varDef
    //  ;
    */
    NodeTypeProgramDecl,




    /*
    //  _              _                _
    // | |_ ___  _ __ | | _____   _____| |
    // | __/ _ \| '_ \| |/ _ \ \ / / _ \ |
    // | || (_) | |_) | |  __/\ V /  __/ |
    //  \__\___/| .__/|_|\___| \_/ \___|_|
    //          |_|
    */

    /*
    // importDecl
    //  : 'import' scope ('->' ident)?
    //  ;
    //
    */
    NodeTypeImportDecl,

    /*
    // typeDef
    //  : 'type' ident ':=' typeDecl
    //  ;
    //
    */
    NodeTypeTypeDef,

    /*
    // funcDef
    //  : funcDefAttrib* 'def' ident funcArgs? funcReturn? funcOuterBody
    //  ;
    */
    NodeTypeFuncDef,

    /*
    // funcDefAttrib
    //  : externAttrib 
    //  | entryAttrib
    //  ;
    */
    NodeTypeFuncDefAttrib,

    /*
    // varDef
    //  : 'let' ident (':' typeDecl)? `:=` expr
    //  | 'let' ident ':' typeDecl (':=' expr)?
    //  ;
    */
   NodeTypeVarDef,




    /*
    //        _   _        _ _           _
    //   __ _| |_| |_ _ __(_) |__  _   _| |_ ___  ___
    //  / _` | __| __| '__| | '_ \| | | | __/ _ \/ __|
    // | (_| | |_| |_| |  | | |_) | |_| | ||  __/\__ \
    //  \__,_|\__|\__|_|  |_|_.__/ \__,_|\__\___||___/
    */

   /*
   // entryAttrib
   //   : 'entry' '(' scope ')'
   //   ;
   //
   */
   NodeTypeAttribEntry,

   /*
   // attributeExtern
   //   : 'extern' '(' externOption? ')'
   //   ;
   //
   // externOption
   //   : 'C'
   //   ;
   */
   NodeTypeAttribExtern,





    /*
    // scope
    //  : ident (':' scope)?
    //  ;
    */
    NodeTypeScope,


    /*
    // typeDecl
    //  : typeAttrib* typeBody ('[' expr ']' | '*')*
    //  ;
    //
    // typeBody
    //  : structDecl 
    //  | tupleDecl 
    //  | nameDecl 
    //  | enumDecl 
    //  | variantDecl
    //  ;
    //
    // namedTypeList
    //  : ident ':' typeDecl (',' namedTypeList)?
    //  ;
    //
    // typeList
    //  : typeDecl (',' typeList)?
    //  ;
    //
    // backingDecl
    //  : ':' typeDecl
    //  ;
    */

    /*
    // structDecl
    //   : '{' namedTypeList? '}'
    //   ;
    */
    NodeTypeStructDecl,

    /*
    // tupleDecl
    //  : '(' typeList? ')'
    //  ;
    */
    NodeTypeTupleDecl,
    
    /*
    // nameDecl
    //  : scope
    //  ;
    */
    NodeTypeNameDecl,

    /*
    // unionDecl
    //  : 'union' structDecl
    //  ;
    */
    NodeTypeUnionDecl,

    /* 
    // enumDecl
    //  : 'enum' backingDecl? '{' enumBody? '}'
    //  ;
    //
    // enumBody
    //  : ident (':=' expr)? (',' enumBody)?
    //  ;
    */
    NodeTypeEnumDecl,

    /*
    // variantDecl
    //  : 'variant' backingDecl? '{' variantBody? '}'
    //  ;
    //
    // variantBody
    //  : ident (':' expr)? '=>' typeDecl (',' variantBody)?
    //  ;
    */
    NodeTypeVariantDecl,


    NodeTypeArrayDecl,
    NodeTypePtrDecl,

    NodeTypeAttribAlign,
    NodeTypeAttribPacked,

    /*
    // expr
    //  : exprBody (accessExpr | subscriptExpr | castExpr | ternaryExpr)?
    //  ;
    //
    // exprBody
    //  : structInitExpr
    //  | tupleInitExpr
    //  | arrayInitExpr
    //  | variantInitExpr
    //  | callExpr
    //  | unaryExpr
    //  | binaryExpr
    //  | ternaryExpr
    //  | branchExpr
    //  | matchExpr
    //  | boolExpr
    //  | intExpr
    //  | floatExpr
    //  | strExpr
    //  | charExpr
    //  | nullExpr
    //  ;
    //
    // namedExprList
    //  : expr ':=' expr (',' namedExprList)?
    //  ;
    //
    // exprList
    //  : expr (',' exprList)?
    //  ;
    */
    NodeTypeNameExpr,

    /*
    // accessExpr
    //  : (('.' | '->' | ':') expr)?
    //  ;
    */
   NodeTypeAccessExpr,

   /*
   // callExpr
   //   : expr '(' callBody? ')'
   //   ;
   //
   // callBody
   //   : expr (',' callBody)?
   //   ;
   */
    NodeTypeCallExpr,

    /*
    // subscriptExpr
    //  : '[' expr ']'
    //  ;
    */
    NodeTypeSubscriptExpr,

    /*
    // structInitExpr
    //  : '{' namedExprList? '}'
    //  ;
    */
    NodeTypeStructInitExpr,

    /*
    // arrayInitExpr
    //  : '[' exprList? ']'
    //  ;
    */
    NodeTypeArrayInitExpr,

    /*
    // tupleInitExpr
    //  : '(' exprList? ')'
    //  ;
    */
    NodeTypeTupleInitExpr,

    /*
    // castExpr
    //  : 'as' typeDecl
    //  ;
    */
    NodeTypeCastExpr,

    /*
    // ternaryExpr
    //  : '?' expr ':' expr
    //  ;
    */
    NodeTypeTernaryExpr,

    /*
    // intExpr
    //  : int
    //  ;
    //
    // floatExpr
    //  : float
    //  ;
    //
    // boolExpr
    //  : 'true'
    //  | 'false'
    //  ;
    //
    // nullExpr
    //  : 'null'
    //  ;
    //
    // strExpr
    //  : string
    //  ;
    //
    // charExpr
    //  : char
    //  ;
    */
    NodeTypeInt,
    NodeTypeFloat,
    NodeTypeBool,
    NodeTypeNull,
    NodeTypeStr,
    NodeTypeChar
} NodeType;

typedef struct Node {
    NodeType type;

    union {
        struct Node* attribPacked;
        struct Node* attribAlign;

        struct {
            vec_node_struct attribs;
            
            union {
                /* map of field name to field type */
                vec_keynode_struct structDecl;
                vec_keynode_struct unionDecl;


                /* array of types */
                vec_node_struct tupleDecl;
                vec_str_struct nameDecl;

                /* pointer to a type */
                struct Node* ptrDecl;
                
                /* fixed size array */
                struct {
                    struct Node* size;
                    struct Node* type;
                } arrayDecl;

                /* map of field name to field value */
                struct {
                    struct Node* backing;
                    vec_keynode_struct fields;
                } enumDecl;

                /* map of field name to field value and field type */
                struct {
                    struct Node* backing;
                    vec_keyvaltype_struct fields;
                } variantDecl;
            } data;
        } typeDecl;

        vec_node_struct tupleExpr;
        vec_node_struct arrayExpr;
        vec_keynode_struct structExpr;
        struct {
            Keyword op;
            struct Node* operand;
        } unaryExpr;

        struct {
            Keyword op;
            struct Node* lhs;
            struct Node* rhs;
        } binaryExpr;

        struct {
            struct Node* cond;
            struct Node* truthy;
            struct Node* falsey;
        } ternaryExpr;

        struct {
            vec_node_struct args;
            struct Node* expr;
        } callExpr;

        struct {
            struct Node* expr;
            struct Node* type;
        } castExpr;

        struct {
            struct Node* expr;
            struct Node* index;
        } subscriptExpr;

        int64_t intExpr;
        double floatExpr;
        char* stringExpr;
        char charExpr;
        int boolExpr;
        void* nullExpr;
        char* nameExpr;

    } data;
} Node;

Node* NewNode(NodeType type)
{
    Node* out;
    out = malloc(sizeof(Node));
    out->type = type;
    return out;
}

typedef struct {
    Lexer* lex;
    /* extra token used sometimes when parsing */
    Token tok;
} Parser;

Parser NewParser(Lexer* lex)
{
    Parser out;
    out.lex = lex;
    out.tok = InvalidToken();
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
