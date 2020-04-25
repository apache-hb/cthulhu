Node* ParseTypeDecl(Parser* parser);
Node* ParseExpr(Parser* parser);

#define LOOP_UNTIL(parser, end, delim, body) \
    if(!ConsumeKeyword(parser, end)) \
    { do { body; } while(ConsumeKeyword(parser, delim)); ExpectKeyword(parser, end); }
