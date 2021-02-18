#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

#ifndef X86
#   define X86(id, str)
#endif

KEY(K_STRUCT, "struct")
KEY(K_UNION, "union")
KEY(K_ENUM, "enum")
KEY(K_USING, "using")
KEY(K_DEF, "def")
KEY(K_VAR, "var")
KEY(K_LET, "let")

KEY(K_FINAL, "final")
KEY(K_IF, "if")
KEY(K_ELSE, "else")
KEY(K_FOR, "for")
KEY(K_WHILE, "while")
KEY(K_WITH, "with")
KEY(K_SWITCH, "switch")
KEY(K_CASE, "case")
KEY(K_BREAK, "break")
KEY(K_CONTINUE, "continue")
KEY(K_RETURN, "return")
KEY(K_CAST, "cast")
KEY(K_VOLATILE, "volatile")
KEY(K_OVERRIDE, "override")
KEY(K_ASM, "asm")
KEY(K_STATIC, "static")
KEY(K_VIRTUAL, "virtual")

OP(K_LPAREN, "(")
OP(K_RPAREN, ")")
OP(K_LSQUARE, "[")
OP(K_RSQUARE, "]")
OP(K_LBRACE, "{")
OP(K_RBRACE, "}")

OP(K_COMMA, ",")
OP(K_SEMI, ";")
OP(K_DOT, ".")
OP(K_DOT2, "..")
OP(K_DOT3, "...")

OP(K_ASSIGN, "=")
OP(K_COLON, ":")
OP(K_COLON2, "::")

OP(K_AT, "@")
OP(K_BEGIN, "!<")
OP(K_END, ">")

#undef KEY
#undef OP
#undef X86
