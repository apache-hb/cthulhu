
/// tree kind defs
#ifndef TREE_KIND
#   define TREE_KIND(ID, NAME)
#endif

TREE_KIND(eTreeType, "type") ///< unresolved type
TREE_KIND(eTreeTypeEmpty, "type-empty") ///< a type that can never be created
TREE_KIND(eTreeTypeUnit, "type-unit") ///< a type with a single value
TREE_KIND(eTreeTypeBool, "type-bool") ///< a type with two values
TREE_KIND(eTreeTypeDigit, "type-digit")
TREE_KIND(eTreeTypeString, "type-string")
TREE_KIND(eTreeTypeClosure, "type-closure") ///< a function pointer type (no captures)
TREE_KIND(eTreeTypePointer, "type-pointer")

TREE_KIND(eTreeTypeStruct, "type-struct")

TREE_KIND(eTreeExprEmpty, "expr-empty") ///< an uninitialized value of a type
TREE_KIND(eTreeExprUnit, "expr-unit")
TREE_KIND(eTreeExprBool, "expr-bool")
TREE_KIND(eTreeExprDigit, "expr-digit")
TREE_KIND(eTreeExprString, "expr-string")

TREE_KIND(eTreeExprLoad, "expr-load")
TREE_KIND(eTreeExprUnary, "expr-unary")
TREE_KIND(eTreeExprBinary, "expr-binary")
TREE_KIND(eTreeExprCompare, "expr-compare")

TREE_KIND(eTreeExprCall, "expr-call")

TREE_KIND(eTreeStmtBlock, "stmt-block")
TREE_KIND(eTreeStmtReturn, "stmt-return")
TREE_KIND(eTreeStmtAssign, "stmt-assign")
TREE_KIND(eTreeStmtLoop, "stmt-loop")
TREE_KIND(eTreeStmtBranch, "stmt-branch")

TREE_KIND(eTreeDeclGlobal, "decl-global")
TREE_KIND(eTreeDeclLocal, "decl-local")
TREE_KIND(eTreeDeclParam, "decl-param")
TREE_KIND(eTreeDeclField, "decl-field")
TREE_KIND(eTreeDeclFunction, "decl-function")
TREE_KIND(eTreeDeclModule, "decl-module")

TREE_KIND(eTreeError, "error")
TREE_KIND(eTreeQualify, "qualify")

#undef TREE_KIND

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

DIGIT_KIND(eDigit8,  "int8")
DIGIT_KIND(eDigit16, "int16")
DIGIT_KIND(eDigit32, "int32")
DIGIT_KIND(eDigit64, "int64")

DIGIT_KIND(eDigitFast8, "fast8")
DIGIT_KIND(eDigitFast16, "fast16")
DIGIT_KIND(eDigitFast32, "fast32")
DIGIT_KIND(eDigitFast64, "fast64")

DIGIT_KIND(eDigitLeast8, "least8")
DIGIT_KIND(eDigitLeast16, "least16")
DIGIT_KIND(eDigitLeast32, "least32")
DIGIT_KIND(eDigitLeast64, "least64")

#undef DIGIT_KIND

#ifndef SIGN_KIND
#   define SIGN_KIND(ID, STR)
#endif

SIGN_KIND(eSignDefault,  "default")
SIGN_KIND(eSignUnsigned, "unsigned")
SIGN_KIND(eSignSigned,   "signed")

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

TYPE_QUALIFIER(eQualDefault, "default", (0 << 0))
TYPE_QUALIFIER(eQualConst, "const", (1 << 0))
TYPE_QUALIFIER(eQualVolatile, "volatile", (1 << 1))
TYPE_QUALIFIER(eQualAtomic, "atomic", (1 << 2))

#undef TYPE_QUALIFIER

/// symbol visibility during linking
#ifndef HLIR_LINKAGE
#   define HLIR_LINKAGE(ID, STR)
#endif

HLIR_LINKAGE(eLinkExport, "exported")
HLIR_LINKAGE(eLinkImport, "imported")
HLIR_LINKAGE(eLinkModule, "internal")
HLIR_LINKAGE(eLinkEntryCli, "entry-cli")
HLIR_LINKAGE(eLinkEntryGui, "entry-gui")

#undef HLIR_LINKAGE

/// symbol visibility during compilation
#ifndef HLIR_VISIBILITY
#    define HLIR_VISIBILITY(ID, NAME)
#endif

HLIR_VISIBILITY(eVisiblePublic, "public")
HLIR_VISIBILITY(eVisiblePrivate, "private")

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

BINARY_OP(eBinaryBitAnd, "bitand", "&")
BINARY_OP(eBinaryBitOr,  "bitor", "|")
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

#ifndef CAST_OP
#   define CAST_OP(ID, NAME)
#endif

CAST_OP(eCastZeroExtend, "zero-extend")
CAST_OP(eCastSignExtend, "sign-extend")
CAST_OP(eCastBit, "bitcast")

#undef CAST_OP

#ifndef HLIR_ARITY
#   define HLIR_ARITY(ID, NAME)
#endif

HLIR_ARITY(eArityFixed, "fixed") // fixed number of arguments
HLIR_ARITY(eArityVariable, "variable") // variable number of arguments, same as C varargs

#undef HLIR_ARITY
