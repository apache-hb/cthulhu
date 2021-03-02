#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

KEY(USING, "using")
KEY(LET, "let")
KEY(VAR, "var")
KEY(COERCE, "coerce")
KEY(TRUE, "true")
KEY(FALSE, "false")
KEY(TEMPLATE, "template")
KEY(DEF, "def")
KEY(STRUCT, "struct")
KEY(UNION, "union")
KEY(ENUM, "enum")
KEY(RETURN, "return")
KEY(WITH, "with")
KEY(WHILE, "while")
KEY(IF, "if")
KEY(ELSE, "else")

OP(LPAREN, "(")
OP(RPAREN, ")")
OP(LSQUARE, "[")
OP(RSQUARE, "]")
OP(LBRACE, "{")
OP(RBRACE, "}")
OP(AT, "@")
OP(SEMI, ";")

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

OP(GT, ">")
OP(GTE, ">=")
OP(LT, "<")
OP(LTE, "<=")

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

OP(NOT, "!")

OP(ASSIGN, "=")
OP(COMMA, ",")
OP(DOT, ".")
OP(DOT2, "..")
OP(DOT3, "...")
OP(ARROW, "->")

OP(COLON, ":")
OP(COLON2, "::")

OP(QUESTION, "?")

#undef KEY
#undef OP
