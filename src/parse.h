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
    NodeTypeTypeDef,

    /*
    // typeAttribute
    //  : '@' typeAttributeBody
    //  ;
    //
    // typeAttributeBody
    //  | attributePacked
    //  | attributeAlign
    //  | attributeNullable
    //  ;
    //
    // attributeNullable
    //  : 'nullable'
    //  ;
    */
    NodeTypeAttributeNullable,

    /*
    // attributePacked
    //  : 'packed' '(' expr ')'
    //  ;
    //
    */
    NodeTypeAttributePacked,

    /*
    // attributeAlign
    //  : 'align' '(' expr ')'
    //  ;
    //
    */
    NodeTypeAttributeAlign,

    /*
    // binaryExpr
    //  : binop expr
    //  ;
    */
    NodeTypeBinaryExpr,

    /*
    // unaryExpr
    //  : expr unaryop expr
    //  ;
    */
    NodeTypeUnaryExpr,

    /*
    // parenExpr
    //  : '(' expr ')'
    //  ;
    */
    NodeTypeParenExpr,

    /*
    // nameExpr
    //  : dotted_name
    //  ;
    */
    NodeTypeNameExpr,

    NodeTypeArrayInitExpr,
    NodeTypeTupleInitExpr,
    NodeTypeStructInitExpr,


    NodeTypeAccessExpr,

    NodeTypeFuncDecl,
    NodeTypeFuncArgs,
    NodeTypeFuncCall,

    NodeTypeIntExpr

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
            vec_node_struct attribs;

            union {
                struct {
                    vec_str_struct names;
                    vec_node_struct types;
                } structDecl;

                struct {
                    struct Node* backing;
                    vec_str_struct names;
                    vec_node_struct values;
                    vec_node_struct types;
                } variantDecl;

                struct {
                    struct Node* backing;
                    vec_str_struct names;
                    vec_node_struct fields;
                } enumDecl;

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

        union {
            struct {
                Keyword op;
                struct Node* operand;
            } unary;

            struct {
                Keyword op;
                struct Node* lhs;
                struct Node* rhs;
            } binary;

            struct {
                struct Node* operand;
            } paren;

            struct {
                int64_t val;
            } integer;

            struct {
                double val;
            } number;

            struct {
                int val;
            } boolean;

            struct {
                Keyword op;
                struct Node* lhs;
                struct Node* rhs;
            } access;

            struct {
                char* name;
            } ident;

            struct {
                vec_node_struct nodes;
            } arrayInit;

            struct {
                vec_str_struct names;
                vec_node_struct values;
            } structInit;

            struct {
                vec_node_struct fields;
            } tupleInit;

            struct {
                struct Node* func;
                vec_node_struct args;
            } call;
        } expr;

        struct {
            char* name;
            struct Node* args;
            struct Node* ret;
            struct Node* body;
        } funcDecl;

        struct {
            vec_str_struct names;
            vec_node_struct args;
        } funcArgs;

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

vec_str_struct ParseNameExpr(Parser* parser)
{
    vec_str_struct vec;
    vec_str_init(&vec);

    do {
        vec_str_append(&vec, NextIdent(parser).data.ident);
    } while(ConsumeKeyword(parser, KeywordColon));

    return vec;
}

Node* ParseTypeDecl(Parser* parser);

Node* ParseExpr(Parser* parser);

Node* ParseTupleInit(Parser* parser)
{
    Node* out;
    vec_node_t values;

    out = NewNode(NodeTypeTupleInitExpr);

    vec_node_init(values);

    do {
        vec_node_append(values, ParseExpr(parser));
    } while(ConsumeKeyword(parser, KeywordComma));

    ExpectKeyword(parser, KeywordRParen);

    out->data.expr.tupleInit.fields = values[0];

    return out;
}

Node* ParseStructInit(Parser* parser)
{
    Node* out;
    vec_str_t names;
    vec_node_t values;
    Token tok;

    out = NewNode(NodeTypeStructInitExpr);
    vec_node_init(values);
    vec_str_init(names);

    do {
        tok = NextIdent(parser);
        vec_str_append(names, tok.data.ident);

        ExpectKeyword(parser, KeywordAssign);

        vec_node_append(values, ParseExpr(parser));
    } while(ConsumeKeyword(parser, KeywordComma));

    ExpectKeyword(parser, KeywordRBrace);

    out->data.expr.structInit.names = names[0];
    out->data.expr.structInit.values = values[0];

    return out;
}

Node* ParseArrayInit(Parser* parser)
{
    Node* out;
    vec_node_t values;
    
    out = NewNode(NodeTypeArrayInitExpr);
    vec_node_init(values);

    do {
        vec_node_append(values, ParseExpr(parser));
    } while(ConsumeKeyword(parser, KeywordComma));

    ExpectKeyword(parser, KeywordRSquare);

    out->data.expr.arrayInit.nodes = values[0];

    return out;
}

Node* ParseAccess(Parser* parser, Node* lhs, Keyword op)
{
    Node* out;

    out = NewNode(NodeTypeAccessExpr);
    out->data.expr.access.op = op;
    out->data.expr.access.lhs = lhs;
    out->data.expr.access.rhs = ParseExpr(parser);

    return out;
}

Node* ParseCall(Parser* parser, Node* lhs)
{
    Node* out;
    vec_node_t args;

    vec_node_init(args);
    out = NewNode(NodeTypeFuncCall);

    if(!ConsumeKeyword(parser, KeywordRParen))
    {
        do {
            vec_node_append(args, ParseExpr(parser));
        } while(ConsumeKeyword(parser, KeywordComma));

        ExpectKeyword(parser, KeywordRParen);
    }

    out->data.expr.call.func = lhs;
    out->data.expr.call.args = args[0];

    return out;
}

Node* ParseExpr(Parser* parser)
{
    Node* out;
    Token tok;

    tok = NextToken(parser);

    if(tok.type == TokenTypeKeyword)
    {
        if(tok.data.keyword == KeywordLParen)
        {
            out = NewNode(NodeTypeParenExpr);
            out->data.expr.paren.operand = ParseExpr(parser);
            ExpectKeyword(parser, KeywordRParen);
        }
        else if(tok.data.keyword == KeywordAdd ||
                tok.data.keyword == KeywordSub ||
                tok.data.keyword == KeywordBitNot ||
                tok.data.keyword == KeywordNot)
        {
            out = NewNode(NodeTypeUnaryExpr);
            out->data.expr.unary.op = tok.data.keyword;
            out->data.expr.unary.operand = ParseExpr(parser);
        }
        else if(tok.data.keyword == KeywordLParen)
        {
            out = ParseTupleInit(parser);
        }
        else if(tok.data.keyword == KeywordLBrace)
        {
            out = ParseStructInit(parser);
        }
        else if(tok.data.keyword == KeywordLSquare)
        {
            out = ParseArrayInit(parser);
        }
        else
        {

        }
    }
    else if(tok.type == TokenTypeIdent)
    {
        out = NewNode(NodeTypeNameExpr);
        out->data.expr.ident.name = tok.data.ident;
    }
    else if(tok.type == TokenTypeChar)
    {

    }
    else if(tok.type == TokenTypeString)
    {
        
    }
    else if(tok.type == TokenTypeInt)
    {
        out = NewNode(NodeTypeIntExpr);
        out->data.expr.integer.val = tok.data.integer;
    }
    else if(tok.type == TokenTypeFloat)
    {

    }
    else
    {
        return NULL;
    }

    while(1)
    {
        if(ConsumeKeyword(parser, KeywordLParen))
        {
            out = ParseCall(parser, out);
            /* expr(expr...) */
        }
        else if(ConsumeKeyword(parser, KeywordColon))
        {
            out = ParseAccess(parser, out, KeywordColon);
            /* name:expr */
        }   
        else if(ConsumeKeyword(parser, KeywordDot))
        {
            out = ParseAccess(parser, out, KeywordDot);
            /* expr.expr */
        }
        else if(ConsumeKeyword(parser, KeywordArrow))
        {
            out = ParseAccess(parser, out, KeywordArrow);
            /* expr->expr */
        }
        else
        {
            break;
        }
    }

    return out;
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
{
    Node* out;
    Node* backing;
    vec_node_t values;
    vec_str_t names;
    Token tok;

    if(ConsumeKeyword(parser, KeywordColon))
    {
        backing = ParseTypeDecl(parser);
    }
    else
    {
        backing = NULL;
    }

    ExpectKeyword(parser, KeywordLBrace);

    vec_node_init(values);
    vec_str_init(names);

    do {
        tok = NextIdent(parser);
        vec_str_append(names, tok.data.ident);

        ExpectKeyword(parser, KeywordAssign);

        vec_node_append(values, ParseExpr(parser));
    } while(ConsumeKeyword(parser, KeywordComma));

    ExpectKeyword(parser, KeywordRBrace);

    out = NewNode(NodeTypeEnum);
    out->data.type.typeDecl.enumDecl.backing = backing;
    out->data.type.typeDecl.enumDecl.names = names[0];
    out->data.type.typeDecl.enumDecl.fields = values[0];

    return out;
}

Node* ParseVariantDecl(Parser* parser) 
{
    Node* out;
    Node* backing;
    vec_node_t fields;
    vec_node_t values;
    vec_str_t names;
    Token tok;

    if(ConsumeKeyword(parser, KeywordColon))
    {
        backing = ParseTypeDecl(parser);
    }
    else
    {
        backing = NULL;
    }

    ExpectKeyword(parser, KeywordLBrace);

    vec_node_init(fields);
    vec_node_init(values);
    vec_str_init(names);

    do {
        tok = NextIdent(parser);
        vec_str_append(names, tok.data.ident);

        if(ConsumeKeyword(parser, KeywordColon))
        {
            vec_node_append(values, ParseExpr(parser));
        }
        else
        {
            vec_node_append(values, NULL);
        }

        ExpectKeyword(parser, KeywordArrow);

        vec_node_append(fields, ParseTypeDecl(parser));

    } while(ConsumeKeyword(parser, KeywordComma));

    ExpectKeyword(parser, KeywordRBrace);
    
    out = NewNode(NodeTypeVariant);
    out->data.type.typeDecl.variantDecl.backing = backing;
    out->data.type.typeDecl.variantDecl.names = names[0];
    out->data.type.typeDecl.variantDecl.types = fields[0];
    out->data.type.typeDecl.variantDecl.values = values[0];

    return out;
}

Node* MakePtr(Node* type)
{
    Node* out;

    out = NewNode(NodeTypePointer);
    out->data.type.typeDecl.ptrDecl.to = type;

    return out;
}

Node* MakeArray(Parser* parser, Node* type)
{
    Node* out;

    out = NewNode(NodeTypeArray);
    out->data.type.typeDecl.arrayDecl.of = type;
    out->data.type.typeDecl.arrayDecl.size = ParseExpr(parser);

    return out;
}

Node* ParseAlignAttrib(Parser* parser)
{
    Node* out;

    ExpectKeyword(parser, KeywordLParen);

    out = NewNode(NodeTypeAttributeAlign);
    out->data.alignAttribute.align = ParseExpr(parser);

    ExpectKeyword(parser, KeywordRParen);

    return out;
}

Node* ParsePackedAttrib(Parser* parser)
{
    Node* out;

    ExpectKeyword(parser, KeywordLParen);

    out = NewNode(NodeTypeAttributePacked);
    out->data.alignAttribute.align = ParseExpr(parser);

    ExpectKeyword(parser, KeywordRParen);

    return out;
}

Node* ParseTypeAttrib(Parser* parser)
{
    Token tok;
    Node* out;

    tok = NextIdent(parser);

    if(strcmp(tok.data.ident, "nullable") == 0)
    {
        out = NewNode(NodeTypeAttributeNullable);
    }
    else if(strcmp(tok.data.ident, "align") == 0)
    {
        out = ParseAlignAttrib(parser);
    }
    else if(strcmp(tok.data.ident, "packed") == 0)
    {
        out = ParsePackedAttrib(parser);
    }
    else
    {
        out = NULL;
    }
    
    return out;
}

Node* ParseTypeDecl(Parser* parser)
{
    Node* out;
    Token tok;
    vec_node_t attribs;

    vec_node_init(attribs);

    while(ConsumeKeyword(parser, KeywordBuiltin))
    {
        vec_node_append(attribs, ParseTypeAttrib(parser));
    }

    tok = NextToken(parser);

    if(tok.type == TokenTypeIdent)
    {
        /* must be a typename */
        out = NewNode(NodeTypeName);
        ParseDottedName(parser, &out->data.type.typeDecl.nameDecl.parts, tok.data.ident);
    }
    else if(tok.type == TokenTypeKeyword)
    {
        switch(tok.data.keyword)
        {
        case KeywordLParen:
            out = ParseTupleDecl(parser); break;
        case KeywordLBrace:
            out = ParseStructDecl(parser); break;
        case KeywordUnion:
            out = ParseUnionDecl(parser); break;
        case KeywordEnum:
            out = ParseEnumDecl(parser); break;
        case KeywordVariant:
            out = ParseVariantDecl(parser); break;
        default:
            out = NULL; break;
        }
    }

    while(1)
    {
        if(ConsumeKeyword(parser, KeywordMul))
        {
            out = MakePtr(out);
        }
        else if(ConsumeKeyword(parser, KeywordLSquare))
        {
            out = MakeArray(parser, out);
        }
        else
        {
            break;
        }
    }

    out->data.type.attribs = attribs[0];

    return out;
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

Node* ParseFuncBody(Parser* parser)
{
    ExpectKeyword(parser, KeywordLBrace);

    ExpectKeyword(parser, KeywordRBrace);

    return NULL;
}

Node* ParseFuncArgs(Parser* parser)
{
    Node* out;
    vec_str_t names;
    vec_node_t args;
    vec_node_init(args);
    vec_str_init(names);

    if(!ConsumeKeyword(parser, KeywordRParen))
    {
        do {
            vec_str_append(names, NextIdent(parser).data.ident);
            ExpectKeyword(parser, KeywordColon);
            vec_node_append(args, ParseTypeDecl(parser));
        } while(ConsumeKeyword(parser, KeywordComma));

        ExpectKeyword(parser, KeywordRParen);
    }

    out = NewNode(NodeTypeFuncArgs);
    out->data.funcArgs.args = args[0];
    out->data.funcArgs.names = names[0];

    return out;
}

Node* ParseFuncDef(Parser* parser)
{
    Token tok;
    Node* out;
    Node* args;
    Node* ret;
    Node* body;

    tok = NextIdent(parser);

    if(ConsumeKeyword(parser, KeywordLParen))
    {
        args = ParseFuncArgs(parser);
    }
    else
    {
        args = NULL;
    }

    if(ConsumeKeyword(parser, KeywordArrow))
    {
        ret = ParseTypeDecl(parser);
    }
    else
    {
        ret = NULL;
    }

    if(ConsumeKeyword(parser, KeywordAssign))
    {
        body = ParseExpr(parser);
    }
    else
    {
        body = ParseFuncBody(parser);
    }

    out = NewNode(NodeTypeFuncDecl);
    out->data.funcDecl.args = args;
    out->data.funcDecl.ret = ret;
    out->data.funcDecl.body = body;
    out->data.funcDecl.name = tok.data.ident;

    return out;
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
        case KeywordDef:
            return ParseFuncDef(parser);
        default:
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}
