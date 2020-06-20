#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

KEY(IMPORT, "import")
KEY(TYPE, "type")
KEY(DEF, "def")
KEY(STRUCT, "struct")
KEY(UNION, "union")
KEY(ENUM, "enum")
KEY(VAR, "var")
KEY(LET, "let")

KEY(IF, "if")
KEY(ELSE, "else")
KEY(WHILE, "while")
KEY(FOR, "for")
KEY(SWITCH, "switch")
KEY(CASE, "case")
KEY(MATCH, "match")
KEY(RETURN, "return")

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

OP(BITAND, "&")
OP(BITANDEQ, "&=")

OP(BITOR, "|")
OP(BITOREQ, "|=")

OP(BITXOR, "^")
OP(BITXOREQ, "^=")

OP(SHL, "<<")
OP(SHLEQ, "<<=")

OP(SHR, ">>")
OP(SHREQ, ">>=")

OP(AND, "&&")
OP(OR, "||")

OP(LSQUARE, "[")
OP(RSQUARE, "]")

OP(LPAREN, "(")
OP(RPAREN, ")")

OP(LBRACE, "{")
OP(RBRACE, "}")

OP(GT, "<")
OP(GTE, "<=")

OP(LT, ">")
OP(LTE, ">=")

OP(EQ, "==")
OP(NEQ, "!=")

OP(NOT, "!")
OP(ASSIGN, ":=")

OP(AT, "@")
OP(HASH, "#")
OP(COMMA, ",")
OP(DOT, ".")
OP(QUESTION, "?")
OP(COLON, ":")
OP(COLON2, "::")
OP(SEMICOLON, ";")


#undef KEY
#undef OP