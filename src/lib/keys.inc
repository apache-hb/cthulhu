// keywords currently in use
#ifndef KEY
#   define KEY(...)
#endif

// reserved keywords
#ifndef RES
#   define RES(...)
#endif

// soft keywords that can be used in attributes
#ifndef SOFT
#   define SOFT(...)
#endif

#ifndef ASM
#   define ASM(...)
#endif

// operators
#ifndef OP
#   define OP(...)
#endif

OP(_add, "+")
OP(_addeq, "+=")
OP(_sub, "-")
OP(_subeq, "-=")
OP(_div, "/")
OP(_diveq, "/=")
OP(_mul, "*")
OP(_muleq, "*=")
OP(_mod, "%")
OP(_modeq, "%=")

OP(_bitnot, "~")
OP(_bitand, "&")
OP(_bitandeq, "&=")
OP(_bitor, "|")
OP(_bitoreq, "|=")
OP(_bitxor, "^")
OP(_bitxoreq, "^=")

OP(_lsquare, "[")
OP(_rsquare, "]")
OP(_lbrace, "{")
OP(_rbrace, "}")
OP(_lparen, "(")
OP(_rparen, ")")

OP(_gt, "<")
OP(_gte, "<=")
OP(_lt, ">")
OP(_lte, ">=")
OP(_eq, "==")
OP(_neq, "!=")
OP(_and, "&&")
OP(_or, "||")

OP(_assign, ":=")
OP(_arrow, "=>")
OP(_question, "?")
OP(_comma, ",")
OP(_dot, ".")
OP(_colon, ":")
OP(_colon2, "::")

KEY(_invalid, "")

KEY(_lsquare2, "[[")
KEY(_rsquare2, "]]")

KEY(_import, "import")
KEY(_def, "def")
KEY(_type, "type")


// inline assembly block with extended intel syntax
SOFT(_asm, "asm")

// modify bits in the resulting binary
// also used to set the generated code type to either 16, 32, or 64
SOFT(_bits, "bits")

// pack a structure to N bits (default 1)
SOFT(_kpacked, "packed")

// align a data structure, value, or function 
SOFT(_kalign, "align")

// make a function inline
SOFT(_kinline, "inline")

// get or set the current physical address
SOFT(_paddr, "paddr")

// get or set the current virtual address
SOFT(_vaddr, "vaddr")

// preserve a register
SOFT(_preserve, "preserve")

// assembly keywords
ASM(_byte, "byte")
ASM(_word, "word")
ASM(_dword, "dword")
ASM(_qword, "qword")


#ifdef KEY
#   undef KEY
#endif

#ifdef RES
#   undef RES
#endif

#ifdef SOFT
#   undef SOFT
#endif

#ifdef ASM
#   undef ASM
#endif

#ifdef OP
#   undef OP
#endif
