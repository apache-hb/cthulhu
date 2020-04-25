#include "parse/data.h"
#include "parse/fwd.h"
#include "parse/exprs.h"
#include "parse/types.h"

#if 0


typedef struct Node {
    NodeType type;


    vec_node_struct attribs;

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

        struct {
            char* name;
            struct Node* type;
            struct Node* expr;
        } varDecl;
    } data;
} Node;


Node* ParseTypeDecl(Parser* parser);
Node* ParseExpr(Parser* parser);



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





Node* ParseUnaryExpr(Parser* parser, Keyword key)
{
    Node* out;

    if(key == KeywordBuiltin)
    {
        /* builtin function */
    }
}

Node* ParseTokenExpr(Parser* parser, Token tok)
{

}

Node* ParseExpr(Parser* parser)
{
    Node* out;
    Token tok;

    tok = NextToken(parser);

    if(tok.type == TokenTypeKeyword)
    {
        /* unary op */
        out = ParseUnaryExpr(parser, tok.data.keyword);
    }
    else
    {
        out = ParseTokenExpr(parser, tok);
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

        ExpectKeyword(parser, KeywordBigArrow);

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

    out->data.attribs = attribs[0];

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

Node* ParseVarDecl(Parser* parser)
{
    Node* out;
    Node* type;
    Node* expr;
    Token tok;

    tok = NextIdent(parser);

    if(ConsumeKeyword(parser, KeywordColon))
    {
        type = ParseTypeDecl(parser);
    }

    if(ConsumeKeyword(parser, KeywordAssign))
    {
        expr = ParseExpr(parser);
    }

    out = NewNode(NodeTypeVarDecl);
    out->data.varDecl.name = tok.data.ident;
    out->data.varDecl.expr = expr;
    out->data.varDecl.type = type;

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
        case KeywordLet:
            return ParseVarDecl(parser);
        default:
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

#endif
