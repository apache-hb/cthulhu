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

// operators
#ifndef OP
#   define OP(...)
#endif

OP(add, "+");
OP(addeq, "+=");
OP(sub, "-");
OP(subeq, "-=");
OP(div, "/");
OP(diveq, "/=");
OP(mul, "*");
OP(muleq, "*=");
OP(mod, "%");
OP(modeq, "%=");

OP(bitnot, "~");
OP(bitand, "&");
OP(bitandeq, "&=");
OP(bitor, "|");
OP(bitoreq, "|=");
OP(bitxor, "^");
OP(bitxoreq, "^=");

OP(lsquare, "[");
OP(rsquare, "]");
OP(lbrace, "{");
OP(rbrace, "}");
OP(lsquare, "[");
OP(rsquare, "]");

OP(gt, "<");
OP(gte, "<=");
OP(lt, ">");
OP(lte, ">=");
OP(eq, "==");
OP(neq, "!=");
OP(and, "&&");
OP(or, "||");

OP(assign, ":=");
OP(arrow, "=>");
OP(question, "?");
OP(comma, ",");
OP(dot, ".");
OP(colon, ":");
OP(colon2, "::");

KEY(kimport, "import");
KEY(kdef, "def");
KEY(ktype, "type");


// inline assembly block with extended intel syntax
SOFT(kasm, "asm");

// modify bits in the resulting binary
// also used to set the generated code type to either 16, 32, or 64
SOFT(bits, "bits");

// pack a structure to N bits (default 1)
SOFT(kpacked, "packed");

// align a data structure, value, or function 
SOFT(kalign, "align");

// make a function inline
SOFT(kinline, "inline");

// get or set the current physical address
SOFT(paddr, "paddr");

// get or set the current virtual address
SOFT(vaddr, "vaddr");

// preserve a register
SOFT(kpreserve, "preserve");

#ifdef KEY
#   undef KEY
#endif

#ifdef RES
#   undef RES
#endif

#ifdef SOFT
#   undef SOFT
#endif

#ifdef OP
#   undef OP
#endif
