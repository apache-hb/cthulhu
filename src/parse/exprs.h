static Node* ParseUnaryExpr(Parser* parser)
{
    Node* out;
    Token tok;

    tok = NextKeyword(parser);
    out = NewNode(NodeTypeUnaryExpr);
    out->data.unaryExpr.op = tok.data.keyword;
    out->data.unaryExpr.operand = ParseExpr(parser);

    return out;
}

static Node* ParseNameExpr(Parser* parser)
{
    Node* out;

    out = NewNode(NodeTypeNameExpr);
    out->data.nameExpr = NextIdent(parser).data.ident;

    return out;
}

static Node* ParseTernaryExpr(Parser* parser, Node* cond)
{
    Node* out;
    out = NewNode(NodeTypeTernaryExpr);
    out->data.ternaryExpr.cond = cond;
    out->data.ternaryExpr.truthy = ParseExpr(parser);

    ExpectKeyword(parser, KeywordColon);

    out->data.ternaryExpr.falsey = ParseExpr(parser);

    return out;
}

static Node* ParseSubscriptExpr(Parser* parser, Node* expr)
{
    Node* out;

    out = NewNode(NodeTypeSubscriptExpr);
    out->data.subscriptExpr.expr = expr;
    out->data.subscriptExpr.index = ParseExpr(parser);

    ExpectKeyword(parser, KeywordRSquare);

    return out;
}

static Node* ParseIntExpr(Parser* parser)
{
    Node* out;

    out = NewNode(NodeTypeInt);
    out->data.intExpr = NextToken(parser).data.integer;

    return out;
}

static Node* ParseFloatExpr(Parser* parser)
{
    Node* out;

    out = NewNode(NodeTypeFloat);
    out->data.floatExpr = NextToken(parser).data.number;

    return out;
}

static Node* ParseStringExpr(Parser* parser)
{
    Node* out;

    out = NewNode(NodeTypeStr);
    out->data.stringExpr = NextToken(parser).data.string;

    return out;
}

static Node* ParseCharExpr(Parser* parser)
{
    Node* out;

    out = NewNode(NodeTypeChar);
    out->data.charExpr = NextToken(parser).data.letter;

    return out;
}

static Node* ParseScopeExpr(Parser* parser, Node* expr)
{
    Node* out;

    out = NewNode(NodeTypeScope);
    out->data.scopeExpr.lhs = expr;
    out->data.scopeExpr.rhs = ParseExpr(parser);

    return out;
}

static Node* ParseAccessExpr(Parser* parser, Node* expr)
{
    Node* out;

    out = NewNode(NodeTypeAccessExpr);
    out->data.accessExpr.lhs = expr;
    out->data.accessExpr.rhs = ParseExpr(parser);

    return out;
}

static Node* ParseDerefExpr(Parser* parser, Node* expr)
{
    Node* out;

    out = NewNode(NodeTypeDerefExpr);
    out->data.accessExpr.lhs = expr;
    out->data.accessExpr.rhs = ParseExpr(parser);

    return out;
}

static Node* ParseCallExpr(Parser* parser, Node* expr)
{
    Node* out;
    vec_keynode_t fields;
    char* name;

    vec_keynode_init(fields);

    LOOP_UNTIL(parser, KeywordRParen, KeywordComma, {
        name = ConsumeIdent(parser);
        if(name)
        {
            ExpectKeyword(parser, KeywordAssign);
        }
        
        vec_keynode_append(fields, MakePair(
            name, ParseExpr(parser)
        ));
    })

    out = NewNode(NodeTypeCallExpr);
    out->data.callExpr.expr = expr;
    out->data.callExpr.args = fields[0];

    return out;
}

static Node* ParseBuildExpr(Parser* parser, Node* expr)
{
    Node* out;
    vec_keynode_t fields;
    char* name;

    vec_keynode_init(fields);

    LOOP_UNTIL(parser, KeywordRBrace, KeywordComma, {
        name = ConsumeIdent(parser);
        if(name)
        {
            ExpectKeyword(parser, KeywordAssign);
        }

        vec_keynode_append(fields, MakePair(
            name, ParseExpr(parser)
        ));
    })

    out = NewNode(NodeTypeBuildExpr);
    out->data.buildExpr.expr = expr;
    out->data.buildExpr.args = fields[0];

    return out;
}

Node* ParseExpr(Parser* parser)
{
    Node* out;
    Token tok;

    tok = NextToken(parser);

    parser->tok = tok;
    switch(tok.type)
    {
    case TokenTypeKeyword:
        out = ParseUnaryExpr(parser);
        break;
    case TokenTypeIdent:
        out = ParseNameExpr(parser);
        break;
    case TokenTypeInt:
        out = ParseIntExpr(parser);
        break;
    case TokenTypeFloat:
        out = ParseFloatExpr(parser);
        break;
    case TokenTypeString:
        out = ParseStringExpr(parser);
        break;
    case TokenTypeChar:
        out = ParseCharExpr(parser);
        break;
    default:
        return NULL;
    }

    while(1)
    {
        if(ConsumeKeyword(parser, KeywordColon))
        {
            out = ParseScopeExpr(parser, out);
        }
        else if(ConsumeKeyword(parser, KeywordDot))
        {
            out = ParseAccessExpr(parser, out);
        }
        else if(ConsumeKeyword(parser, KeywordArrow))
        {
            out = ParseDerefExpr(parser, out);
        }
        else if(ConsumeKeyword(parser, KeywordLSquare))
        {
            out = ParseSubscriptExpr(parser, out);
        }
        else if(ConsumeKeyword(parser, KeywordLParen))
        {
            out = ParseCallExpr(parser, out);
        }
        else if(ConsumeKeyword(parser, KeywordLBrace))
        {
            out = ParseBuildExpr(parser, out);
        }
        else if(ConsumeKeyword(parser, KeywordQuestion))
        {
            out = ParseTernaryExpr(parser, out);
        }
        else
        {
            break;
        }
    }

    return out;
}

Node* ParseFuncBody(Parser* parser)
{
    /* TODO */
    (void)parser;
    return NULL;    
}
