#define LOOP_UNTIL(parser, end, delim, body) \
    if(!ConsumeKeyword(parser, end)) \
    { do { body; } while(ConsumeKeyword(parser, delim)); ExpectKeyword(parser, end); }


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


Node* ParseTypeName(Parser* parser)
{
    Node* out;
    Token tok;
    vec_str_t parts;

    vec_str_init(parts);

    do {
        tok = NextIdent(parser);
        vec_str_append(parts, tok.data.ident);
    } while(ConsumeKeyword(parser, KeywordColon));

    out = NewNode(NodeTypeNameDecl);
    out->data.typeDecl.data.nameDecl = parts[0];

    return out;
}



Node* ParseAlignAttrib(Parser* parser)
{
    Node* out;

    ExpectKeyword(parser, KeywordLParen);

    out = NewNode(NodeTypeAttribAlign);
    out->data.attribAlign = ParseExpr(parser);

    ExpectKeyword(parser, KeywordRParen);

    return out;
}

Node* ParsePackedAttrib(Parser* parser)
{
    Node* out;

    ExpectKeyword(parser, KeywordLParen);

    out = NewNode(NodeTypeAttribPacked);
    out->data.attribPacked = ParseExpr(parser);

    ExpectKeyword(parser, KeywordRParen);

    return out;
}

Node* ParseTypeAttrib(Parser* parser)
{
    Node* out;
    Token tok;

    tok = NextIdent(parser);

    if(strcmp(tok.data.ident, "align") == 0)
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

Node* MakePtr(Node* type)
{
    Node* out;

    out = NewNode(NodeTypePtrDecl);
    out->data.typeDecl.data.ptrDecl = type;

    return out;
}

Node* MakeArray(Parser* parser, Node* type)
{
    Node* out;

    out = NewNode(NodeTypeArrayDecl);
    out->data.typeDecl.data.arrayDecl.type = type;
    out->data.typeDecl.data.arrayDecl.size = ParseExpr(parser);

    ExpectKeyword(parser, KeywordRSquare);

    return out;
}

Node* ParseStructDecl(Parser* parser)
{
    Node* out;
    Node* type;
    Token tok;
    KeyNode pair;
    vec_keynode_t fields;

    vec_keynode_init(fields);

    LOOP_UNTIL(parser, KeywordRParen, KeywordComma, {
        tok = NextIdent(parser);

        ExpectKeyword(parser, KeywordColon);

        type = ParseTypeDecl(parser);

        pair.key = tok.data.ident;
        pair.node = type;

        vec_keynode_append(fields, pair);
    });

    out = NewNode(NodeTypeStructDecl);
    out->data.typeDecl.data.structDecl = fields[0];

    return out;
}

Node* ParseTupleDecl(Parser* parser)
{
    Node* out;
    vec_node_t fields;

    vec_node_init(fields);

    LOOP_UNTIL(parser, KeywordRParen, KeywordComma, {
        vec_node_append(fields, ParseTypeDecl(parser));
    });

    out = NewNode(NodeTypeTupleDecl);
    out->data.typeDecl.data.tupleDecl = fields[0];

    return out;
}

Node* ParseUnionDecl(Parser* parser)
{
    Node* out;
    ExpectKeyword(parser, KeywordLParen);

    out = ParseStructDecl(parser);

    out->type = NodeTypeUnionDecl;
    out->data.typeDecl.data.unionDecl = out->data.typeDecl.data.structDecl;

    return out;
}

Node* ParseEnumDecl(Parser* parser)
{
    Node* out;
    Node* backing;
    vec_keynode_t fields;

    backing = ConsumeKeyword(parser, KeywordColon) ? ParseTypeDecl(parser) : NULL;

    vec_keynode_init(fields);

    ExpectKeyword(parser, KeywordLBrace);
    LOOP_UNTIL(parser, KeywordRBrace, KeywordComma, {
        vec_keynode_append(fields, MakePair(
            NextIdent(parser).data.ident,
            ConsumeKeyword(parser, KeywordAssign) ? ParseExpr(parser) : NULL
        ));
    });

    out = NewNode(NodeTypeEnumDecl);
    out->data.typeDecl.data.enumDecl.backing = backing;
    out->data.typeDecl.data.enumDecl.fields = fields[0];

    return out;
}

Node* ParseVariantDecl(Parser* parser)
{
    Node* out;
    Node* backing;
    vec_keyvaltype_t fields;
    
    Token tok;
    Node* type;
    Node* val;

    vec_keyvaltype_init(fields);

    backing = ConsumeKeyword(parser, KeywordColon) ? ParseTypeDecl(parser) : NULL;



    ExpectKeyword(parser, KeywordLBrace);
    LOOP_UNTIL(parser, KeywordRBrace, KeywordComma, {
        tok = NextIdent(parser);
        val = ConsumeKeyword(parser, KeywordColon) ? ParseExpr(parser) : NULL;

        ExpectKeyword(parser, KeywordBigArrow);

        type = ParseTypeDecl(parser);

        vec_keyvaltype_append(fields, MakeKeyValType(
            tok.data.ident,
            val,
            type
        ));
    });



    out = NewNode(NodeTypeVariantDecl);

    out->data.typeDecl.data.variantDecl.backing = backing;
    out->data.typeDecl.data.variantDecl.fields = fields[0];

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
        parser->tok = tok;
        out = ParseTypeName(parser);
    }
    else if(tok.type == TokenTypeKeyword)
    {
        switch(tok.data.keyword)
        {
        case KeywordLBrace: out = ParseStructDecl(parser); break;
        case KeywordLParen: out = ParseTupleDecl(parser); break;
        case KeywordUnion: out = ParseUnionDecl(parser); break;
        case KeywordEnum: out = ParseEnumDecl(parser); break;
        case KeywordVariant: out = ParseVariantDecl(parser); break;
        default:
            printf("oh no\n");
            out = NULL;
            break;
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

    out->data.typeDecl.attribs = attribs[0];

    return out;
}
