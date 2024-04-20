// SPDX-License-Identifier: GPL-3.0-only

#ifndef PL0_TYPE
#   define PL0_TYPE(id, name)
#endif

PL0_TYPE(ePl0Digit, "digit")
PL0_TYPE(ePl0Ident, "ident")

PL0_TYPE(ePl0Odd, "odd")
PL0_TYPE(ePl0Unary, "unary")
PL0_TYPE(ePl0Binary, "binary")
PL0_TYPE(ePl0Compare, "compare")

PL0_TYPE(ePl0Assign, "assign")
PL0_TYPE(ePl0Call, "call")
PL0_TYPE(ePl0Branch, "branch")
PL0_TYPE(ePl0Loop, "loop")
PL0_TYPE(ePl0Print, "print")
PL0_TYPE(ePl0Stmts, "stmts")

PL0_TYPE(ePl0Value, "value")
PL0_TYPE(ePl0Procedure, "procedure")

PL0_TYPE(ePl0Import, "import")
PL0_TYPE(ePl0Module, "module")

#undef PL0_TYPE

#ifndef DECL_TAG
#   define DECL_TAG(id, init, str)
#endif

DECL_TAG(ePl0TagValues, = eSemaValues, "values")
DECL_TAG(ePl0TagTypes, = eSemaTypes, "types")
DECL_TAG(ePl0TagProcs, = eSemaProcs, "procedures")
DECL_TAG(ePl0TagModules, = eSemaModules, "modules")

DECL_TAG(ePl0TagImportedValues, /* empty */, "imported values")
DECL_TAG(ePl0TagImportedProcs, /* empty */, "imported procedures")

#undef DECL_TAG
