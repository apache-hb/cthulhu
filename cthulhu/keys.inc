#ifndef KEY
#   define KEY(id, str, flags)
#endif

#ifndef OP
#   define OP(id, str)
#endif

#ifndef FLAG
#   define FLAG(name, bit)
#endif

FLAG(LF_CORE, 1)
FLAG(LF_ASM, 2)

/* declaration keywords */
KEY(K_IMPORT, "import", LF_CORE)
KEY(K_DEF, "def", LF_CORE)
KEY(K_ALIAS, "alias", LF_CORE)
KEY(K_STRUCT, "struct", LF_CORE)
KEY(K_ENUM, "enum", LF_CORE)
KEY(K_UNION, "union", LF_CORE)
KEY(K_OBJECT, "object", LF_CORE)
KEY(K_VAR, "var", LF_CORE)
KEY(K_TRAIT, "trait", LF_CORE)

/* control keywords */
KEY(K_IF, "if", LF_CORE)
KEY(K_ELSE, "else", LF_CORE)
KEY(K_WHILE, "while", LF_CORE)
KEY(K_FOR, "for", LF_CORE)
KEY(K_RETURN, "return", LF_CORE)
KEY(K_BRANCH, "branch", LF_CORE)
KEY(K_COERCE, "coerce", LF_CORE)
KEY(K_BREAK, "break", LF_CORE)
KEY(K_CONTINUE, "continue", LF_CORE)

KEY(K_INT, "int", LF_ASM)
KEY(K_UD2, "ud2", LF_ASM)
KEY(K_NOP, "nop", LF_ASM)

/* language operators */
OP(K_AT, "@")
OP(K_TBEGIN, "!<")
OP(K_TEND, ">")
OP(K_SEMI, ";")
OP(K_COLON, ":")
OP(K_COLON2, "::")
OP(K_PTR, "->")
OP(K_ARROW, "=>")
OP(K_COMMA, ",")
OP(K_DOT, ".")
OP(K_QUESTION, "?")
OP(K_ASSIGN, "=")

/* logical operators */
OP(K_NOT, "!")
OP(K_NEQ, "!=")
OP(K_EQ, "==")

OP(K_GT, "<")
OP(K_GTE, "<=")

OP(K_LT, ">")
OP(K_LTE, ">=")

OP(K_AND, "&&")
OP(K_OR, "||")

/* bitwise operators */
OP(K_XOR, "^")
OP(K_XOREQ, "^=")

OP(K_BITAND, "&")
OP(K_BITANDEQ, "&=")

OP(K_BITOR, "|")
OP(K_BITOREQ, "|=")

OP(K_SHL, "<<")
OP(K_SHLEQ, "<<=")

OP(K_SHR, ">>")
OP(K_SHREQ, ">>=")

OP(K_BITNOT, "~")

/* math operators */
OP(K_ADD, "+")
OP(K_ADDEQ, "+=")

OP(K_SUB, "-")
OP(K_SUBEQ, "-=")

OP(K_DIV, "/")
OP(K_DIVEQ, "/=")

OP(K_MUL, "*")
OP(K_MULEQ, "*=")

OP(K_MOD, "%")
OP(K_MODEQ, "%=")

/* extra operators */
OP(K_LPAREN, "(")
OP(K_RPAREN, ")")
OP(K_LSQUARE, "[")
OP(K_RSQUARE, "]")
OP(K_LBRACE, "{")
OP(K_RBRACE, "}")

#undef KEY
#undef OP
#undef FLAG
