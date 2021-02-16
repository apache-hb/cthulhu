#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

#ifndef X86
#   define X86(id, str)
#endif

KEY(DEF, "def")
KEY(STRUCT, "struct")
KEY(UNION, "union")
KEY(ENUM, "enum")
KEY(USING, "using")
KEY(LET, "let")
KEY(VAR, "var")
KEY(RETURN, "return")
KEY(WHILE, "while")
KEY(FOR, "for")
KEY(IF, "if")
KEY(ELSE, "else")
KEY(WITH, "with")
KEY(SWITCH, "switch")
KEY(CASE, "case")

KEY(VIRTUAL, "virtual")
KEY(STATIC, "static")
KEY(FINAL, "final")
KEY(ASM, "asm")
KEY(VOLATILE, "volatile")
KEY(OVERRIDE, "override")
KEY(CAST, "cast")



OP(LPAREN, "(")
OP(RPAREN, ")")
OP(LSQUARE, "[")
OP(RSQUARE, "]")
OP(LBRACE, "{")
OP(RBRACE, "}")
OP(AT, "@")
OP(COMMA, ",")
OP(DOT, ".") 
OP(DOT2, "..") // range loops
OP(DOT3, "...") // import wildcards and varargs

OP(BEGIN, "!<")
OP(END, ">")

OP(ASSIGN, "=")
OP(ARROW, "->")

OP(SEMI, ";")
OP(COLON, ":")
OP(COLON2, "::")

OP(GT, ">")
OP(GTE, ">=")
OP(LT, "<")
OP(LTE, "<=")

OP(AND, "&&")
OP(OR, "||")
OP(EQ, "==")
OP(NEQ, "!=")
OP(NOT, "!")


OP(ADD, "+")
OP(ADDEQ, "+=")
OP(SUB, "-")
OP(SUBEQ, "-=")
OP(DIV, "/")
OP(DIVEQ, "/=")
OP(MUL, "*")
OP(MULEQ, "*=")
OP(MOD, "%")
OP(MODEQ, "%=")


OP(SHL, "<<")
OP(SHLEQ, "<<=")
OP(SHR, ">>")
OP(SHREQ, ">>=")

OP(BITAND, "&")
OP(BITANDEQ, "&=")
OP(BITOR, "|")
OP(BITOREQ, "|=")
OP(BITXOR, "^")
OP(BITXOREQ, "^=")
OP(BITNOT, "~")


#undef KEY
#undef OP
#undef X86
