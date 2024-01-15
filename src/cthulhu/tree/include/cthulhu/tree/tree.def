/// tree kind defs
#ifndef TREE_KIND
#   define TREE_KIND(ID, NAME)
#endif

///
/// types
///

TREE_KIND(eTreeTypeEmpty, "type_empty") ///< a type that can never be created
TREE_KIND(eTreeTypeUnit, "type_unit") ///< a type with a single value
TREE_KIND(eTreeTypeBool, "type_bool") ///< a type with two values
TREE_KIND(eTreeTypeOpaque, "type_opaque")
TREE_KIND(eTreeTypeDigit, "type_digit")
TREE_KIND(eTreeTypeClosure, "type_closure") ///< a function pointer type (no captures)

TREE_KIND(eTreeTypeReference, "type_reference") ///< a reference to an object
TREE_KIND(eTreeTypePointer, "type_pointer") ///< a pointer to an object
TREE_KIND(eTreeTypeArray, "type_array") ///< an array of objects

TREE_KIND(eTreeTypeStruct, "type_struct")
TREE_KIND(eTreeTypeUnion, "type_union")
TREE_KIND(eTreeTypeEnum, "type_enum")

///
/// expressions
/// these yield a value
///

TREE_KIND(eTreeExprEmpty, "expr_empty") ///< an uninitialized value of a type
TREE_KIND(eTreeExprUnit, "expr_unit")
TREE_KIND(eTreeExprBool, "expr_bool")
TREE_KIND(eTreeExprDigit, "expr_digit")
TREE_KIND(eTreeExprString, "expr_string")

TREE_KIND(eTreeExprCast, "expr_cast")
TREE_KIND(eTreeExprLoad, "expr_load")
TREE_KIND(eTreeExprReference, "expr_reference")
TREE_KIND(eTreeExprAddress, "expr_address")
TREE_KIND(eTreeExprUnary, "expr_unary")
TREE_KIND(eTreeExprBinary, "expr_binary")
TREE_KIND(eTreeExprCompare, "expr_compare")

TREE_KIND(eTreeExprField, "expr_field") ///< get a reference to a field
TREE_KIND(eTreeExprOffset, "expr_offset")

TREE_KIND(eTreeExprCall, "expr_call")

///
/// statements
/// these cannot yield a value
///

TREE_KIND(eTreeStmtBlock, "stmt_block")
TREE_KIND(eTreeStmtReturn, "stmt_return")
TREE_KIND(eTreeStmtAssign, "stmt_assign")
TREE_KIND(eTreeStmtLoop, "stmt_loop")
TREE_KIND(eTreeStmtBranch, "stmt_branch")
TREE_KIND(eTreeStmtJump, "stmt_jump") ///< loop related jumps

///
/// declarations
///

TREE_KIND(eTreeDeclGlobal, "decl_global")
TREE_KIND(eTreeDeclLocal, "decl_local")
TREE_KIND(eTreeDeclParam, "decl_param")
TREE_KIND(eTreeDeclCase, "decl_case")
TREE_KIND(eTreeDeclField, "decl_field")
TREE_KIND(eTreeDeclFunction, "decl_function")
TREE_KIND(eTreeDeclModule, "decl_module")

TREE_KIND(eTreeType, "unresolved_type") ///< unresolved type
TREE_KIND(eTreeError, "error") ///< an error of some sort

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

SIGN_KIND(eSignDefault,  "default") ///< dealing with C interop
SIGN_KIND(eSignUnsigned, "unsigned")
SIGN_KIND(eSignSigned,   "signed")

#undef SIGN_KIND

#ifndef TREE_BUILTIN
#   define TREE_BUILTIN(ID, STR)
#endif

TREE_BUILTIN(eBuiltinSizeOf,    "sizeof")
TREE_BUILTIN(eBuiltinAlignOf,   "alignof")
TREE_BUILTIN(eBuiltinOffsetOf,  "offsetof")
TREE_BUILTIN(eBuiltinUUIDOf,    "uuidof") // __uuidof from msvc

#undef TREE_BUILTIN

#ifndef TYPE_QUALIFIER
#   define TYPE_QUALIFIER(ID, STR, BIT)
#endif

TYPE_QUALIFIER(eQualUnknown, "unknown", (0 << 0))
TYPE_QUALIFIER(eQualConst, "const", (1 << 0))
TYPE_QUALIFIER(eQualMutable, "mutable", (1 << 1))
TYPE_QUALIFIER(eQualVolatile, "volatile", (1 << 2))
TYPE_QUALIFIER(eQualAtomic, "atomic", (1 << 3))
TYPE_QUALIFIER(eQualRestrict, "restrict", (1 << 4))

#undef TYPE_QUALIFIER

/// symbol visibility during linking
#ifndef TREE_LINKAGE
#   define TREE_LINKAGE(ID, STR)
#endif

TREE_LINKAGE(eLinkExport, "exported")
TREE_LINKAGE(eLinkImport, "imported")
TREE_LINKAGE(eLinkModule, "internal")
TREE_LINKAGE(eLinkEntryCli, "entry_cli")
TREE_LINKAGE(eLinkEntryGui, "entry_gui")
TREE_LINKAGE(eLinkPixelShader, "pixel_shader")
TREE_LINKAGE(eLinkVertexShader, "vertex_shader")
TREE_LINKAGE(eLinkComputeShader, "compute_shader")

#undef TREE_LINKAGE

/// symbol visibility during compilation
#ifndef TREE_VISIBILITY
#    define TREE_VISIBILITY(ID, NAME)
#endif

TREE_VISIBILITY(eVisiblePublic, "public")
TREE_VISIBILITY(eVisiblePrivate, "private")

#undef TREE_VISIBILITY

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

CAST_OP(eCastZeroExtend, "zero_extend")
CAST_OP(eCastSignExtend, "sign_extend")
CAST_OP(eCastBit, "bit_cast")

#undef CAST_OP

#ifndef TREE_ARITY
#   define TREE_ARITY(ID, NAME)
#endif

TREE_ARITY(eArityFixed, "fixed") // fixed number of arguments
TREE_ARITY(eArityVariable, "variable") // variable number of arguments, same as C varargs

#undef TREE_ARITY

#ifndef TREE_JUMP
#   define TREE_JUMP(ID, NAME)
#endif

TREE_JUMP(eJumpBreak, "break")
TREE_JUMP(eJumpContinue, "continue")

#undef TREE_JUMP
