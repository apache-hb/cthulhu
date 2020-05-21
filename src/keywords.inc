#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

// toplevel keywords
KEY(DEF, "def")
KEY(INCLUDE, "include")
KEY(TYPE, "type")

// type keywords
KEY(STRUCT, "struct")
KEY(UNION, "union")
KEY(ANY, "any")
KEY(ENUM, "enum")
KEY(CONST, "const")

// control flow
KEY(WHILE, "while")
KEY(FOR, "for")
KEY(BREAK, "break")
KEY(CONTINUE, "continue")
KEY(IF, "if")
KEY(ELSE, "else")
KEY(RETURN, "return")

KEY(AS, "as")

// math ops
OP(ADD, "+")
OP(ADDEQ, "+=")

OP(SUB, "-")
OP(SUBEQ, "-=")

OP(MUL, "*")
OP(MULEQ, "*=")

OP(DIV, "/")
OP(DIVEQ, "/=")

OP(MOD, "%")
OP(MODEQ, "%")

// logical ops

OP(NOT, "!")

OP(AND, "&&")
OP(OR, "||")

OP(GT, ">")
OP(GTE, ">=")

OP(LT, "<")
OP(LTE, "<=")

OP(EQ, "==")
OP(NEQ, "!=")

// bitwise ops

OP(BITNOT, "~")

OP(XOR, "^")
OP(XOREQ, "^=")

OP(BITAND, "&")
OP(BITANDEQ, "&=")

OP(BITOR, "|")
OP(BITOREQ, "|=")

OP(SHL, "<<")
OP(SHLEQ, "<<=")

OP(SHR, ">>")
OP(SHREQ, ">>=")

// syntax sugar

OP(ARROW, "->")
OP(BIGARROW, "=>")
OP(COLON, ":")
OP(COLON2, "::")
OP(ASSIGN, ":=")
OP(AT, "@")
OP(DOT, ".")
OP(COMMA, ",")
OP(QUESTION, "?")

OP(LPAREN, "(")
OP(RPAREN, ")")

OP(LBRACE, "{")
OP(RBRACE, "}")

OP(LSQUARE, "[")
OP(RSQUARE, "]")

#undef KEY
#undef OP
#undef ASM
