Node* ParseUnaryExpr(Parser* parser)
{
    Node* out;

    return out;
}

Node* ParseNameExpr(Parser* parser)
{
    Node* out;

    out = NewNode(NodeTypeNameExpr);
    out->data.nameExpr = NextIdent(parser).data.ident;

    return out;
}

Node* ParseTerneryExpr(Parser* parser, Node* cond)
{
    Node* out;
    out = NewNode(NodeTypeTernaryExpr);
    out->data.ternaryExpr.cond = cond;
    out->data.ternaryExpr.truthy = ParseExpr(parser);

    ExpectKeyword(parser, KeywordColon);

    out->data.ternaryExpr.falsey = ParseExpr(parser);

    return out;
}

Node* ParseSubscriptExpr(Parser* parser, Node* expr)
{
    Node* out;

    out = NewNode(NodeTypeSubscriptExpr);
    out->data.subscriptExpr.expr = expr;
    out->data.subscriptExpr.index = ParseExpr(parser);

    ExpectKeyword(parser, KeywordRSquare);

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

        }
        else if(ConsumeKeyword(parser, KeywordDot))
        {

        }
        else if(ConsumeKeyword(parser, KeywordArrow))
        {

        }
        else if(ConsumeKeyword(parser, KeywordLSquare))
        {

        }
        else if(ConsumeKeyword(parser, KeywordLParen))
        {

        }
        else if(ConsumeKeyword(parser, KeywordLBrace))
        {

        }
        else if(ConsumeKeyword(parser, KeywordQuestion))
        {
            out = ParseTerneryExpr(parser, out);
        }
        else
        {
            break;
        }
    }

    return out;
}
