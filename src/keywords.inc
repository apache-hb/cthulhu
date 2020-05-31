#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

#ifndef ASM
#   define ASM(id, str)
#endif

// toplevel keywords
KEY(IMPORT, "import")
KEY(TYPE, "type")
KEY(DEF, "def")
KEY(TEMPLATE, "template")
KEY(OPERATOR, "operator")

// type keywords
KEY(STRUCT, "struct")
KEY(UNION, "union")
KEY(OBJECT, "object")
KEY(CONST, "const")
KEY(ENUM, "enum")

// control flow
KEY(IF, "if")
KEY(ELSE, "else")
KEY(SWITCH, "switch")
KEY(CASE, "case")
KEY(DEFAULT, "default")
KEY(MATCH, "match")
KEY(BREAK, "break")
KEY(CONTINUE, "continue")
KEY(FOR, "for")
KEY(WHILE, "while")
KEY(RETURN, "return")

OP(LPAREN, "(")
OP(RPAREN, ")")
OP(LSQUARE, "[")
OP(RSQUARE, "]")
OP(LBRACE, "{")
OP(RBRACE, "}")

OP(DIV, "/")
OP(DIVEQ, "/=")

// assembly instructions
ASM(PUSH, "push")
ASM(POP, "pop")
ASM(CALL, "call")
ASM(RET, "ret")
ASM(JMP, "jmp")
ASM(MOV, "mov")

#undef KEY
#undef OP
#undef ASM
