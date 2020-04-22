#define VECTOR_TYPE char*
#define VECTOR_NAME str
#include "vector.h"

#define VECTOR_TYPE void*
#define VECTOR_NAME node
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
    //  | ptrDecl
    //  ;
    //

    // structDecl
    //  : '{' structBody '}'
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
    //  : '(' tupleBody ')'
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
    //  : 'union' '{' unionBody '}'
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
    //  : 'enum' enumBacking? '{' enumBody '}'
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
    //  : '[' typeDecl ':' expr ']'
    //  ;
    */
    NodeTypeArray,

    /*
    // variantDecl
    //  : 'variant' variantTypeBacking? '{' variantBody '}'
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
    // ptrDecl
    //  : '*' typeDecl
    //  ;
    */
    NodeTypePointer,

    /*
    // typeDef
    //  : 'type' Ident ':=' typeDecl
    //  ;
    //
    */
    NodeTypeTypeDef

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
    if(tok.type == TokenTypeKeyword && tok.data.keyword == key)
    {
        parser->tok = InvalidToken();
        return 1;
    }

    return 0;
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
                    vec_str_struct names;
                    vec_node_struct types;
                } structDecl;

                struct {
                    vec_node_struct fields;
                } tupleDecl;

                struct {
                    vec_str_t parts;
                } nameDecl;

                struct {
                    vec_str_struct names;
                    vec_node_struct types;
                } unionDecl;

                struct {
                    struct Node* of;
                    struct Node* size;
                } arrayDecl;

                struct {
                    struct Node* to;
                } ptrDecl;
            } typeDecl;
        } type;

        struct { 
            char* name;
            struct Node* typeDecl;
        } typeDef;
    } data;
} Node;

Node* NewNode(NodeType type)
{
    Node* node = malloc(sizeof(Node));
    node->type = type;
    return node;
}

void ParseDottedName(Parser* parser, vec_str_t* vec, char* extra)
{
    Token tok;
    vec_str_init(*vec);

    if(extra)
    {
        vec_str_append(*vec, extra);
        tok = NextKeyword(parser);
        if(tok.data.keyword != KeywordColon)
        {
            parser->tok = tok;
            return;
        }
    }

    while(1)
    {
        tok = NextIdent(parser);
        vec_str_append(*vec, tok.data.ident);

        tok = NextKeyword(parser);
        if(tok.data.keyword != KeywordColon)
        {
            parser->tok = tok;
            break;
        }
    }
}

Node* ParseTypeDecl(Parser* parser);

Node* ParseExpr(Parser* parser)
{ (void)parser;
    /* TODO */
    return NULL;
}

Node* ParseTupleDecl(Parser* parser) 
{
    vec_node_t nodes;
    Node* out;

    Token tok;

    vec_node_init(nodes);

    do
    {
        vec_node_append(nodes, ParseTypeDecl(parser));

        tok = NextKeyword(parser);
    }
    while(tok.data.keyword == KeywordComma);

    parser->tok = tok;

    ExpectKeyword(parser, KeywordRParen);

    out = NewNode(NodeTypeTuple);
    out->data.type.typeDecl.tupleDecl.fields = nodes[0];

    return out;
}

Node* ParseStructDecl(Parser* parser) 
{
    vec_str_t names;
    vec_node_t nodes;
    Node* out;
    Token tok;

    vec_str_init(names);
    vec_node_init(nodes);

    do 
    {
        tok = NextIdent(parser);
        vec_str_append(names, tok.data.ident);
        
        ExpectKeyword(parser, KeywordColon);

        vec_node_append(nodes, ParseTypeDecl(parser));

        tok = NextKeyword(parser);
    }
    while(tok.data.keyword == KeywordComma);

    parser->tok = tok;

    ExpectKeyword(parser, KeywordRBrace);

    out = NewNode(NodeTypeStruct);
    
    out->data.type.typeDecl.structDecl.names = names[0];
    out->data.type.typeDecl.structDecl.types = nodes[0];

    return out;
}

Node* ParseUnionDecl(Parser* parser) 
{
    vec_str_t names;
    vec_node_t fields;
    Token tok;
    Node* out;
    
    ExpectKeyword(parser, KeywordLBrace);

    vec_node_init(fields);
    vec_str_init(names);

    do
    {
        tok = NextIdent(parser);
        vec_str_append(names, tok.data.ident);
        ExpectKeyword(parser, KeywordColon);
        vec_node_append(fields, ParseTypeDecl(parser));
        tok = NextKeyword(parser);
    }
    while(tok.data.keyword == KeywordComma);

    parser->tok = tok;

    ExpectKeyword(parser, KeywordRBrace);

    out = NewNode(NodeTypeUnion);
    out->data.type.typeDecl.unionDecl.names = names[0];
    out->data.type.typeDecl.unionDecl.types = fields[0];

    return out;
}

Node* ParseEnumDecl(Parser* parser) 
{ (void)parser;
    /* Node* backing;
    vec_str_t names;
    Node* values; */

    return NULL;
}

Node* ParseVariantDecl(Parser* parser) 
{ (void)parser;
    /* Node* backing;
    Node* fields;
    Node* values;
    vec_str_t names; */

    return NULL;
}

Node* ParseArrayDecl(Parser* parser) 
{
    Node* wraps;
    Node* size;
    Node* out;

    wraps = ParseTypeDecl(parser);
    ExpectKeyword(parser, KeywordColon);
    size = ParseExpr(parser);

    out = NewNode(NodeTypeArray);
    out->data.type.typeDecl.arrayDecl.of = wraps;
    out->data.type.typeDecl.arrayDecl.size = size;

    return out;
}

Node* ParsePtrDecl(Parser* parser)
{
    Node* out;
    Node* to;

    to = ParseTypeDecl(parser);
    out = NewNode(NodeTypePointer);
    out->data.type.typeDecl.ptrDecl.to = to;

    return out;
}

Node* ParseTypeDecl(Parser* parser)
{
    Node* type;
    Token tok;

    tok = NextToken(parser);

    if(tok.type == TokenTypeIdent)
    {
        /* must be a typename */
        type = NewNode(NodeTypeName);
        ParseDottedName(parser, &type->data.type.typeDecl.nameDecl.parts, tok.data.ident);
        return type;
    }
    else if(tok.type == TokenTypeKeyword)
    {
        switch(tok.data.keyword)
        {
        case KeywordLParen:
            return ParseTupleDecl(parser);
        case KeywordLBrace:
            return ParseStructDecl(parser);
        case KeywordUnion:
            return ParseUnionDecl(parser);
        case KeywordEnum:
            return ParseEnumDecl(parser);
        case KeywordVariant:
            return ParseVariantDecl(parser);
        case KeywordLSquare:
            return ParseArrayDecl(parser);
        case KeywordMul:
            return ParsePtrDecl(parser);
        default:
            return NULL;    
        }
    }

    /* oh no */
    return NULL;
}

Node* ParseTypeDef(Parser* parser)
{
    Node* node;
    Node* type;
    char* name;

    name = NextIdent(parser).data.ident;
    ExpectKeyword(parser, KeywordAssign);
    type = ParseTypeDecl(parser);

    node = NewNode(NodeTypeTypeDef);
    node->data.typeDef.name = name;
    node->data.typeDef.typeDecl = type;

    return node;
}

Node* ParseImport(Parser* parser)
{
    Node* node;
    
    node = NewNode(NodeTypeImportDecl);
    ParseDottedName(parser, &node->data.importDecl.path, NULL);

    if(ConsumeKeyword(parser, KeywordArrow))
    {
        node->data.importDecl.alias = NextIdent(parser).data.ident;
        parser->tok = InvalidToken();
    }
    else
    {
        node->data.importDecl.alias = NULL;
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
        tok = NextToken(parser);
    }

    if(tok.type == TokenTypeKeyword)
    {
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
    else
    {
        return NULL;
    }
}
