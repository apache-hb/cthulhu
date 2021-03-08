#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

KEY(USING, "using")
KEY(VAR, "var")
KEY(TRUE, "true")
KEY(FALSE, "false")
KEY(COERCE, "coerce")

OP(LPAREN, "(")
OP(RPAREN, ")")
OP(LSQUARE, "[")
OP(RSQUARE, "]")
OP(LBRACE, "{")
OP(RBRACE, "}")

OP(SEMI, ";")
OP(COMMA, ",")
OP(DOT, ".")
OP(DOT2, "..")
OP(DOT3, "...")
OP(QUESTION, "?")
OP(ASSIGN, "=")
OP(AT, "@")
OP(NOT, "!")
OP(ARROW, "->")

OP(COLON, ":")
OP(COLON2, "::")

OP(ADD, "+")
OP(ADDEQ, "+=")

OP(SUB, "-")
OP(SUBEQ, "-=")

OP(MUL, "*")
OP(MULEQ, "*=")

OP(DIV, "/")
OP(DIVEQ, "/=")

OP(MOD, "%")
OP(MODEQ, "%=")

OP(BEGIN, "!<")
OP(END, ">")

OP(EQ, "==")
OP(NEQ, "!=")

OP(AND, "&&")
OP(OR, "||")

OP(SHL, "<<")
OP(SHLEQ, "<<=")
OP(SHR, ">>")
OP(SHREQ, ">>=")

OP(BITAND, "&")
OP(BITANDEQ, "&=")

OP(BITOR, "|")
OP(BITOREQ, "|=")

OP(XOR, "^")
OP(XOREQ, "^=")

OP(FLIP, "~")

OP(LT, "<")
OP(LTE, "<=")
OP(GT, ">")
OP(GTE, ">=")

#undef KEY
#undef OP
