Node* ParseTypeDecl(Parser* parser);
Node* ParseExpr(Parser* parser);

#define LOOP_UNTIL(parser, end, delim, body) \
    if(!ConsumeKeyword(parser, end)) \
    { do { body; } while(ConsumeKeyword(parser, delim)); ExpectKeyword(parser, end); }

#define CONSUME_UNTIL(parser, end, body) \
    if(!ConsumeKeyword(parser, end)) \
    { do { body; } while(!ConsumeKeyword(parser, end)); ExpectKeyword(parser, end); }
    