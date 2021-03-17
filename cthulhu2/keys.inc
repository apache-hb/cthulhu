#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef ASM
#   define ASM(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

KEY(RECORD, "record")
KEY(VARIANT, "variant")
KEY(ASM, "asm")
KEY(TRUE, "true")
KEY(FALSE, "false")

// prefix asm instructions with A to prevent clashing
ASM(AADD, "add")
ASM(AXOR, "xor")

OP(ADD, "+")
OP(ADDEQ, "+=")
OP(SUB, "-")
OP(SUBEQ, "-=")

OP(SHL, "<<")
OP(SHLEQ, "<<=")
OP(SHR, ">>")
OP(SHREQ, ">>=")

OP(BEGIN, "!<")
OP(END, ">")

OP(NOT, "!")
OP(NEQ, "!=")

OP(LT, "<")
OP(LTE, "<=")
OP(GT, ">")
OP(GTE, ">=")

#undef KEY
#undef ASM
#undef OP
