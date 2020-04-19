#define VECTOR_TYPE char*
#define VECTOR_NAME str
#include "vector.h"

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

typedef enum {
    /*
    // name
    //  : Ident ('::' name)?
    //  ;
    //
    */
    NodeTypeDottedName,

    /*
    // importDecl 
    //  : 'import' name ('->' Ident)?
    //  ;
    //
    */
    NodeTypeImportDecl,

    /*
    // typeDecl
    //  : typeAttribute+ typeDeclBody
    //  ;
    //
    // typeDeclBody
    //  : structDecl
    //  | tupleDecl
    //  | unionDecl
    //  | enumDecl
    //  | arrayDecl
    //  | variantDecl
    //  | nameDecl
    //  ;
    //

    // structDecl
    //  : '{' structBody? '}'
    //  ;
    //
    // structBody
    //  : Ident ':' typeDecl (',' structBody)?
    //  ;
    //
    */
    NodeTypeStruct,

    /*
    // tupleDecl
    //  : '(' tupleBody? ')'
    //  ;
    //
    // tupleBody
    //  : typeDecl (',' tupleBody)?
    //  ;
    //
    */
    NodeTypeTuple,

    /*
    // unionDecl
    //  : 'union' '{' unionBody? '}'
    //  ;
    //
    // unionBody
    //  : Ident ':' typeDecl (',' unionBody)?
    //  ;
    //
    */
    NodeTypeUnion,

    /*
    // enumDecl
    //  : 'enum' enumBacking? '{' enumBody? '}'
    //  ;
    //
    // enumBacking
    //  : ':' typeDecl
    //  ;
    //
    // enumBody
    //  : Ident ':=' expr
    //  ;
    //
    */
    NodeTypeEnum,

    /*
    // arrayDecl
    //  : '[' typeDecl ':' expr? ']'
    //  ;
    */
    NodeTypeArray,

    /*
    // variantDecl
    //  : 'variant' variantTypeBacking? '{' variantBody? '}'
    //  ;
    //
    // variantTypeBacking
    //  : ':' typeDecl
    //  ;
    //
    // variantBody
    //  : Ident variantBodyBacking? '->' typeDecl (',' variantBody)
    //  ;
    //
    // variantBodyBacking
    //  : ':' expr
    //  ;
    //
    */
    NodeTypeVariant,

    /*
    // nameDecl
    //  : name
    //  ;
    //
    */
    NodeTypeName,

    /*
    // typeDef
    //  : 'type' Ident ':=' typeDecl
    //  ;
    //
    */
    NodeTypeTypeDef,

    /*
    // typeAttribute
    //  : '@' typeAttributeBody
    //  ;
    //
    // typeAttributeBody
    //  | attributePacked
    //  | attributeAlign
    //  ;

    // attributePacked
    //  : 'packed' '(' expr ')'
    //  ;
    //
    // NodeTypeAttributePacked,

    // attributeAlign
    //  : 'align' '(' expr ')'
    //  ;
    //
    // NodeTypeAttributeAlign,
    */
} NodeType;

Token NextKeyword(Parser* parser)
{
    Token tok = LexerNext(parser->lex);

    if(tok.type != TokenTypeKeyword)
    {
        printf("expected keyword\n");
        exit(500);
    }

    return tok;
}

Token NextIdent(Parser* parser)
{
    Token tok = LexerNext(parser->lex);

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
        printf("incorrect keyword found\n");
        exit(500);
    }
}

typedef struct Node {
    NodeType type;

    union {
        struct {
            struct Node* width;
        } packedAttribute;

        struct {
            struct Node* align;
        } alignAttribute;

        struct {
            vec_str_t path;
            char* alias;
        } importDecl;

        struct {
            int attribCount;
            struct Node* attribs;

            union {
                struct {
                    vec_str_t names;
                    struct Node* types;
                } structDecl;

                struct {
                    int count;
                    struct Node* types;
                } tupleDecl;

                struct {
                    vec_str_t parts;
                } nameDecl;
            };
        } typeDecl;

        struct { 
            char* name;
            struct Node* typeDecl;
        } typeDef;
    };
} Node;

Node* NewNode(NodeType type)
{
    Node* node = malloc(sizeof(Node));
    node->type = type;
    return node;
}

void ParseDottedName(Parser* parser, vec_str_t* vec)
{
    vec_str_init(*vec);

    for(;;)
    {
        Token tok = NextIdent(parser);
        vec_str_append(*vec, tok.data.ident);

        tok = NextKeyword(parser);
        if(tok.data.keyword != KeywordColon)
        {
            parser->tok = tok;
            break;
        }
    }
}

Node* ParseTypeDef(Parser* parser)
{
    /* TODO: all this */
    return NULL;
}

Node* ParseImport(Parser* parser)
{
    Node* node = NewNode(NodeTypeImportDecl);
    ParseDottedName(parser, &node->importDecl.path);

    if(parser->tok.data.keyword == KeywordArrow)
    {
        node->importDecl.alias = NextIdent(parser).data.ident;
        parser->tok = InvalidToken();
    }
    else
    {
        node->importDecl.alias = NULL;
    }

    return node;
}

Node* ParserNext(Parser* parser)
{
    Token tok;
    if(IsValidToken(parser->tok))
    {
        tok = parser->tok;
        parser->tok = InvalidToken();
        if(tok.type != TokenTypeKeyword)
        {
            printf("invalid lookahead token\n");
            exit(500);
        }
    }
    else
    {
        tok = NextKeyword(parser);
    }

    switch(tok.data.keyword)
    {
    case KeywordImport:
        return ParseImport(parser);
    case KeywordType:
        return ParseTypeDef(parser);
        break;
    default:
        return NULL;
        break;
    }
}