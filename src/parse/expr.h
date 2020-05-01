Node* ParseType(Parser*);
Node* ParseExpr(Parser*);

Node* ParseTupleInit(Parser* parser)
{
    Node* out;

    vec_node_t items;

    vec_node_init(items);
    REPEAT(parser, KeywordRParen, KeywordComma, {
        vec_node_append(items, ParseExpr(parser));
    })

    out = NewNode(NodeTypeTupleInit);
    out->data._tuple = items[0];

    return out;
}

Node* ParseStructInit(Parser* parser)
{
    Node* out;

    Token tok;
    Node* expr;

    vec_keynode_t items;

    vec_keynode_init(items);

    REPEAT(parser, KeywordRBrace, KeywordComma, {
        tok = NextIdent(parser);
        ExpectKeyword(parser, KeywordAssign);
        expr = ParseExpr(parser);

        vec_keynode_append(items, MakePair(tok.data.ident, expr));
    })

    out = NewNode(NodeTypeStructInit);
    out->data._struct = items[0];

    return out;
}

Node* ParseArrayInit(Parser* parser)
{
    Node* out;

    vec_node_t items;

    REPEAT(parser, KeywordRSquare, KeywordComma, {
        vec_node_append(items, ParseExpr(parser));
    })

    out = NewNode(NodeTypeArrayInit);
    out->data._array = items[0];

    return out;
}

Node* MakeBinary(Parser* parser, Token tok, Node* expr)
{
    Node* out;

    out = NewNode(NodeTypeBinary);
    out->data._binary.op = tok.data.keyword;
    out->data._binary.lhs = expr;
    out->data._binary.rhs = ParseExpr(parser);

    return out;
}

Node* ParseExprBody(Parser* parser)
{
    Node* out;
    Token tok;

    tok = NextToken(parser);

    if(tok.type == TokenTypeKeyword)
    {
        switch(tok.data.keyword)
        {
            /* unary operators */
        case KeywordSub:
        case KeywordAdd:
            /* deref pointers */
        case KeywordMul:
        case KeywordNot:
        case KeywordBitNot:
            out = NewNode(NodeTypeUnary);
            out->data._unary.op = tok.data.keyword;
            out->data._unary.expr = ParseExpr(parser);
            break;

            /* tuple init */
        case KeywordLParen:
            out = ParseTupleInit(parser);
            break;

            /* struct init */
        case KeywordLBrace:
            out = ParseStructInit(parser);
            break;

            /* array init */
        case KeywordLSquare:
            out = ParseArrayInit(parser);
            break;

        default:
            /* other stuff is invalid */
            break;
        }
    }
    else if(tok.type == TokenTypeIdent)
    {   
        out = NewNode(NodeTypeIdent);
        out->data._ident = tok.data.ident;
    }
    else if(tok.type == TokenTypeInt)
    {
        out = NewNode(NodeTypeInt);
        out->data._int = tok.data.integer;
    }

    tok = PeekToken(parser, TokenTypeKeyword);

    if(IsValidToken(tok))
    {
        out = MakeBinary(parser, tok, out);
    }

    return out;
}

Node* ParseExpr(Parser* parser)
{
    Node* out;

    out = ParseExprBody(parser);

    return out;
}

Node* ParseStruct(Parser* parser)
{
    Node* out;
    vec_keynode_t fields;

    Token tok;
    Node* type;

    vec_keynode_init(fields);

    REPEAT(parser, KeywordRBrace, KeywordComma, {
        tok = NextIdent(parser);
        ExpectKeyword(parser, KeywordColon);
        type = ParseType(parser);

        vec_keynode_append(fields, MakePair(tok.data.ident, type));
    })

    out = NewNode(NodeTypeStruct);
    out->data.type._struct = fields[0];

    return out;
}

Node* ParseTuple(Parser* parser)
{
    Node* out;
    vec_node_t fields;

    Node* type;

    vec_node_init(fields);
    REPEAT(parser, KeywordRParen, KeywordComma, {
        type = ParseType(parser);
        vec_node_append(fields, type);
    })

    out = NewNode(NodeTypeTuple);
    out->data.type._tuple = fields[0];

    return out;
}

Node* ParseUnion(Parser* parser)
{
    Node* out;
    vec_keynode_t fields;

    Token tok;
    Node* type;

    vec_keynode_init(fields);

    ExpectKeyword(parser, KeywordLBrace);
    REPEAT(parser, KeywordRBrace, KeywordComma, {
        tok = NextIdent(parser);
        ExpectKeyword(parser, KeywordColon);
        type = ParseType(parser);

        vec_keynode_append(fields, MakePair(tok.data.ident, type));
    })

    out = NewNode(NodeTypeUnion);
    out->data.type._union = fields[0];

    return out;
}

Node* ParseEnum(Parser* parser)
{
    Node* out;
    vec_keynode_t fields;

    Token tok;
    Node* val;

    vec_keynode_init(fields);

    ExpectKeyword(parser, KeywordLBrace);
    REPEAT(parser, KeywordRBrace, KeywordComma, {
        tok = NextIdent(parser);
        val = ConsumeKeyword(parser, KeywordAssign) ? ParseExpr(parser) : NULL;

        vec_keynode_append(fields, MakePair(tok.data.ident, val));
    })

    out = NewNode(NodeTypeEnum);
    out->data.type._enum = fields[0];

    return out;
}

Node* ParseVariant(Parser* parser)
{
    Node* out;
    vec_keyvaltype_t fields;

    Token tok;
    Node* val;
    Node* type;

    vec_keyvaltype_init(fields);

    ExpectKeyword(parser, KeywordLBrace);
    REPEAT(parser, KeywordRBrace, KeywordComma, {
        tok = NextIdent(parser);

        val = ConsumeKeyword(parser, KeywordColon) ? ParseExpr(parser) : NULL;

        ExpectKeyword(parser, KeywordBigArrow);
        type = ParseType(parser);

        vec_keyvaltype_append(fields, MakeKeyValType(
            tok.data.ident, val, type
        ));
    })

    out = NewNode(NodeTypeVariant);
    out->data.type._variant = fields[0];

    return out;
}

Node* ParseTypeName(Parser* parser)
{
    Node* out;
    vec_str_t parts;
    Token tok;

    vec_str_init(parts);

    do
    {
        tok = NextIdent(parser);
        vec_str_append(parts, tok.data.ident);
    }
    while(ConsumeKeyword(parser, KeywordColon));

    out = NewNode(NodeTypeName);
    out->data.type._name = parts[0];

    return out;
}


Node* ParseTypeAttribute(Parser* parser)
{
    Node* out;
    Node* expr;
    Token tok;

    tok = NextIdent(parser);

    ExpectKeyword(parser, KeywordLParen);
    expr = ParseExpr(parser);
    ExpectKeyword(parser, KeywordRParen);

    if(strcmp(tok.data.ident, "packed") == 0)
    {
        out = NewNode(NodeTypePacked);
        out->data._packed = expr;
    }
    else if(strcmp(tok.data.ident, "align") == 0)
    {
        out = NewNode(NodeTypeAlign);
        out->data._align = expr;
    }
    else
    {
        out = NULL;
    }
    
    return out;
}

Node* MakePtr(Node* ptr)
{
    Node* out;

    out = NewNode(NodeTypePtr);
    out->data.type._ptr = ptr;

    return out;
}

Node* MakeArray(Parser* parser, Node* it)
{
    Node* out;
    Node* expr;

    expr = ParseExpr(parser);

    out = NewNode(NodeTypeArray);
    out->data.type._array.size = expr;
    out->data.type._array.type = it;

    return out;
}

Node* ParseType(Parser* parser)
{
    Node* out;
    Token tok;

    vec_node_t attribs;
    
    vec_node_init(attribs);
    while(ConsumeKeyword(parser, KeywordBuiltin))
    {
        vec_node_append(attribs, ParseTypeAttribute(parser));
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
        case KeywordLBrace: out = ParseStruct(parser); break;
        case KeywordLParen: out = ParseTuple(parser); break;
        case KeywordUnion: out = ParseUnion(parser); break;
        case KeywordEnum: out = ParseEnum(parser); break;
        case KeywordVariant: out = ParseVariant(parser); break;
        default: break;
        }
    }
    else
    {
        out = NULL;
    }

    while(1)
    {
        if(ConsumeKeyword(parser, KeywordLSquare))
        {
            out = MakeArray(parser, out);
        }
        else if(ConsumeKeyword(parser, KeywordMul))
        {
            out = MakePtr(out);
        }
        else
        {
            break;
        }
    }

    return out;
}



Node* ParseAttribute(Parser* parser)
{
    (void)parser;
    return NULL;
}


Node* ParseFuncdef(Parser* parser)
{
    Node* out;

    (void)parser;
    out = NULL;

    return out;
}

Node* ParseTypedef(Parser* parser)
{
    Node* out;
    char* name;
    Node* type;

    name = NextIdent(parser).data.ident;

    ExpectKeyword(parser, KeywordAssign);

    type = ParseType(parser);

    out = NewNode(NodeTypeTypedef);
    out->data._typedef.name = name;
    out->data._typedef.type = type;

    return out;
}

Node* ParseBody(Parser* parser)
{
    Node* out;
    Token tok;

    Node* attrib;
    vec_node_t attribs;

    vec_node_init(attribs);
    while(ConsumeKeyword(parser, KeywordBuiltin))
    {
        attrib = ParseAttribute(parser);
        vec_node_append(attribs, attrib);
    }

    tok = NextToken(parser);

    if(tok.type != TokenTypeKeyword)
    {
        return NULL;
    }

    switch(tok.data.keyword)
    {
    case KeywordDef: 
        out = ParseFuncdef(parser);
        break;
    case KeywordType: 
        out = ParseTypedef(parser);
        break;
    default:
        out = NULL;
        break;
    }

    return out;
}

Node* ParseImport(Parser* parser)
{
    Node* out;
    vec_str_t parts;
    char* alias;
    Token tok;

    vec_str_init(parts);
    do
    {
        tok = NextIdent(parser);
        vec_str_append(parts, tok.data.ident);
    }
    while(ConsumeKeyword(parser, KeywordColon));

    alias = ConsumeKeyword(parser, KeywordArrow) ? NextIdent(parser).data.ident : NULL;

    out = NewNode(NodeTypeImport);
    out->data._import.path = parts[0];
    out->data._import.alias = alias;

    return out;
}

Node* ParseProgram(Parser* parser)
{
    Node* out;
    Node* import;
    Node* part;
    char* name;
    vec_node_t imports;
    vec_node_t body;

    if(ConsumeKeyword(parser, KeywordBuiltin))
    {
        /* 
        // if the first statement is @name("filename")
        // then put that into the parse tree for stuff like 
        // generating other languages and specifying the name 
        // of the generated file
        */
        ExpectIdent(parser, "name");
        ExpectKeyword(parser, KeywordLParen);
        name = ExpectToken(parser, TokenTypeString).data.string;
        ExpectKeyword(parser, KeywordRParen);
    }
    else
    {
        name = NULL;
    }

    vec_node_init(imports);
    vec_node_init(body);

    while(ConsumeKeyword(parser, KeywordImport))
    {
        import = ParseImport(parser);
        vec_node_append(imports, import);
    }

    do
    {
        part = ParseBody(parser);
        vec_node_append(body, part);
    }
    while(part);

    out = NewNode(NodeTypeProgram);
    out->data._program.name = name;
    out->data._program.imports = imports[0];
    out->data._program.body = body[0];

    return out;
}