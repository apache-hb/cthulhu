#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

KEY(USING, "using")
KEY(SCOPE, "scope")

OP(LPAREN, "(")
OP(RPAREN, ")")
OP(LSQUARE, "[")
OP(RSQUARE, "]")
OP(LBRACE, "{")
OP(RBRACE, "}")
OP(AT, "@")

OP(ADD, "+")
OP(ADDEQ, "+=")

OP(MUL, "*")
OP(MULEQ, "*=")

OP(BEGIN, "!<")
OP(END, ">")

OP(EQ, "==")
OP(NEQ, "!=")

OP(GT, ">")
OP(GTE, ">=")
OP(LT, "<")
OP(LTE, "<=")

OP(SHL, "<<")
OP(SHLEQ, "<<=")
OP(SHR, ">>")
OP(SHREQ, ">>=")

OP(NOT, "!")

OP(ASSIGN, "=")
OP(COMMA, ",")
OP(DOT, ".")
OP(DOT2, "..")
OP(DOT3, "...")

OP(COLON, ":")
OP(COLON2, "::")

#undef KEY
#undef OP
