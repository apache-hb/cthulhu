
/// hlir tree kind defs
#ifndef HLIR_KIND
#   define HLIR_KIND(ID, NAME)
#endif

// scalar types
HLIR_KIND(eHlirType,   "meta-type")
HLIR_KIND(eHlirDigit,  "digit-type")
HLIR_KIND(eHlirString, "string-type")
HLIR_KIND(eHlirBool,   "bool-type")
HLIR_KIND(eHlirVoid,   "void-type")

// user defined scalar types
HLIR_KIND(eHlirArray,   "array-type")
HLIR_KIND(eHlirPointer, "pointer-type")
HLIR_KIND(eHlirClosure, "closure-type")

// user defined record types
HLIR_KIND(eHlirStruct, "struct-type")
HLIR_KIND(eHlirUnion,  "union-type")
HLIR_KIND(eHlirEnum,   "enum-type")
HLIR_KIND(eHlirAlias,  "alias-type")

HLIR_KIND(eHlirRecordField, "record-field")
HLIR_KIND(eHlirEnumField,   "enum-field")

// literal values
HLIR_KIND(eHlirDigitLiteral,  "digit-literal")
HLIR_KIND(eHlirBoolLiteral,   "bool-literal")
HLIR_KIND(eHlirStringLiteral, "string-literal")

// expressions
HLIR_KIND(eHlirLoad,    "load") // a load from an address, equivilent to a C rvalue
HLIR_KIND(eHlirUnary,   "unary")
HLIR_KIND(eHlirBinary,  "binary") // integer aritmatic
HLIR_KIND(eHlirCompare, "compare") // all boolean logic
HLIR_KIND(eHlirCall,    "call")

HLIR_KIND(eHlirAccess, "access") // aggregate member access
HLIR_KIND(eHlirIndex,  "index") // array access

HLIR_KIND(eHlirBuiltin, "builtin")

// statements
HLIR_KIND(eHlirStmts,    "stmts") // statement list
HLIR_KIND(eHlirBranch,   "branch")
HLIR_KIND(eHlirLoop,     "loop")
HLIR_KIND(eHlirBreak,    "break")
HLIR_KIND(eHlirContinue, "continue")

HLIR_KIND(eHlirAssign, "assign")
HLIR_KIND(eHlirReturn, "return")

// declarations
HLIR_KIND(eHlirGlobal,   "global")
HLIR_KIND(eHlirFunction, "function")
HLIR_KIND(eHlirForward,  "forward")
HLIR_KIND(eHlirModule,   "module")

HLIR_KIND(eHlirParam, "param")
HLIR_KIND(eHlirLocal, "local")

HLIR_KIND(eHlirError, "error")

#undef HLIR_KIND

#ifndef DIGIT_KIND
#   define DIGIT_KIND(ID, STR)
#endif

DIGIT_KIND(eDigitChar,  "char")
DIGIT_KIND(eDigitShort, "short")
DIGIT_KIND(eDigitInt,   "int")
DIGIT_KIND(eDigitLong,  "long")

DIGIT_KIND(eDigitPtr,  "intptr")
DIGIT_KIND(eDigitSize, "intsize")
DIGIT_KIND(eDigitMax,  "intmax")

#undef DIGIT_KIND

#ifndef SIGN_KIND
#   define SIGN_KIND(ID, STR)
#endif

SIGN_KIND(eUnsigned, "unsigned")
SIGN_KIND(eSigned,   "signed")

#undef SIGN_KIND

#ifndef HLIR_BUILTIN
#   define HLIR_BUILTIN(ID, STR)
#endif

HLIR_BUILTIN(eBuiltinSizeOf, "sizeof")
HLIR_BUILTIN(eBuiltinAlignOf, "alignof")

#undef HLIR_BUILTIN

#ifndef TYPE_QUALIFIER
#   define TYPE_QUALIFIER(ID, STR, BIT)
#endif

TYPE_QUALIFIER(eQualConst, "const", (1 << 0))
TYPE_QUALIFIER(eQualVolatile, "volatile", (1 << 1))
TYPE_QUALIFIER(eQualAtomic, "atomic", (1 << 2))

#undef TYPE_QUALIFIER

/// symbol visibility during linking
#ifndef HLIR_LINKAGE
#   define HLIR_LINKAGE(ID, STR)
#endif

HLIR_LINKAGE(eLinkExported, "exported")
HLIR_LINKAGE(eLinkImported, "imported")
HLIR_LINKAGE(eLinkInternal, "internal")
HLIR_LINKAGE(eLinkEntryCli, "entry-cli")
HLIR_LINKAGE(eLinkEntryGui, "entry-gui")

#undef HLIR_LINKAGE

/// symbol visibility during compilation
#ifndef HLIR_VISIBILITY
#    define HLIR_VISIBILITY(ID, NAME)
#endif

HLIR_VISIBILITY(eVisiblePublic, "public")
HLIR_VISIBILITY(eVisibleModule, "module")

#undef HLIR_VISIBILITY

#ifndef UNARY_OP
#   define UNARY_OP(ID, NAME, SYMBOL)
#endif

UNARY_OP(eUnaryAbs,  "abs",  "+")
UNARY_OP(eUnaryNeg,  "neg",  "-")
UNARY_OP(eUnaryFlip, "flip", "~")
UNARY_OP(eUnaryNot,  "not",  "!")

#undef UNARY_OP

#ifndef BINARY_OP
#   define BINARY_OP(ID, NAME, SYMBOL)
#endif

BINARY_OP(eBinaryAdd, "add", "+")
BINARY_OP(eBinarySub, "sub", "-")
BINARY_OP(eBinaryMul, "mul", "*")
BINARY_OP(eBinaryDiv, "div", "/")
BINARY_OP(eBinaryRem, "rem", "%")

BINARY_OP(eBinaryBitAnd, "and", "&")
BINARY_OP(eBinaryBitOr,  "bor", "|")
BINARY_OP(eBinaryXor,    "xor", "^")
BINARY_OP(eBinaryShl,    "shl", ">>")
BINARY_OP(eBinaryShr,    "shr", "<<")

#undef BINARY_OP

#ifndef COMPARE_OP
#   define COMPARE_OP(ID, NAME, SYMBOL)
#endif

COMPARE_OP(eCompareAnd, "and", "&&")
COMPARE_OP(eCompareOr,  "or",  "||")

COMPARE_OP(eCompareEq,  "eq",  "==")
COMPARE_OP(eCompareNeq, "neq", "!=")
COMPARE_OP(eCompareLt,  "lt",  "<")
COMPARE_OP(eCompareLte, "lte", "<=")
COMPARE_OP(eCompareGt,  "gt",  ">")
COMPARE_OP(eCompareGte, "gte", ">=")

#undef COMPARE_OP
