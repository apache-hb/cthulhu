// SPDX-License-Identifier: LGPL-3.0-only

#ifndef TREE_TAG
#   define TREE_TAG(ID, NAME, BIT)
#endif

TREE_TAG(eTagNone,    "none",       (0 << 0)) ///< no tags
TREE_TAG(eTagName,    "name",       (1 << 0)) ///< the node type can have a name
TREE_TAG(eTagQual,    "qual",       (1 << 1)) ///< this node type can have type qualifiers
TREE_TAG(eTagIsType,  "is_a_type",  (1 << 2)) ///< this node type is a type
TREE_TAG(eTagHasType, "has_a_type", (1 << 3)) ///< this node type has a type it evaluates to
TREE_TAG(eTagStorage, "storage",    (1 << 4)) ///< this node type can have a storage class
TREE_TAG(eTagEval,    "eval",       (1 << 5)) ///< this node type can have an evaluation model

#undef TREE_TAG

/// tree kind defs
#ifndef TREE_KIND
#   define TREE_KIND(ID, NAME, TAGS)
#endif

///
/// types
///

/// an empty type is a type that can never be created
/// can be used to denote a function that wont return
TREE_KIND(eTreeTypeEmpty,   "type_empty",   eTagName | eTagQual | eTagIsType)

/// a type that has a single possible value
/// equivalent to `void` in C
TREE_KIND(eTreeTypeUnit,    "type_unit",    eTagName | eTagQual | eTagIsType)

/// boolean logic type
TREE_KIND(eTreeTypeBool,    "type_bool",    eTagName | eTagQual | eTagIsType)

/// an opaque, possibly typed, pointer
/// equivalent to both `void*` as well as a pointer to an incomplete type
TREE_KIND(eTreeTypeOpaque,  "type_opaque",  eTagName | eTagQual | eTagIsType)

/// a digit with a fixed width and sign
TREE_KIND(eTreeTypeDigit,   "type_digit",   eTagName | eTagQual | eTagIsType)

/// a function pointer type (no captures)
TREE_KIND(eTreeTypeClosure, "type_closure", eTagName | eTagQual | eTagIsType)

/// an immutable string
/// this is a special sort of array that can be implicitly cast to a pointer
/// or array
TREE_KIND(eTreeTypeString,  "type_string",  eTagName | eTagQual | eTagIsType)

/// a reference to an object
/// equivalent to an lvalue in C or a `T&` in C++
TREE_KIND(eTreeTypeReference, "type_reference",   eTagName | eTagQual | eTagIsType)

/// a pointer to an object
/// plain old `T*`
TREE_KIND(eTreeTypePointer,   "type_pointer",     eTagName | eTagQual | eTagIsType)

/// an array of objects that may optionally have a fixed size
/// either `T[]` or `T[N]`
TREE_KIND(eTreeTypeArray,     "type_array",       eTagName | eTagQual | eTagIsType)

/// a class type
TREE_KIND(eTreeTypeClass,   "type_class",   eTagName | eTagIsType)

/// a struct type
TREE_KIND(eTreeTypeStruct,  "type_struct",  eTagName | eTagIsType)

/// a union type
TREE_KIND(eTreeTypeUnion,   "type_union",   eTagName | eTagIsType)

/// an enum
TREE_KIND(eTreeTypeEnum,    "type_enum",    eTagName | eTagIsType)

/// a discriminated union, or sum type
TREE_KIND(eTreeTypeVariant, "type_variant", eTagName | eTagIsType)

/// a type alias
TREE_KIND(eTreeTypeAlias,   "type_alias",   eTagName | eTagIsType | eTagHasType)

///
/// expressions
/// these yield a value
///

/// sizeof expression
TREE_KIND(eTreeExprSizeOf,  "expr_sizeof",  eTagHasType)

/// alignof expression
TREE_KIND(eTreeExprAlignOf, "expr_alignof", eTagHasType)

/// offsetof expression
TREE_KIND(eTreeExprOffsetOf, "expr_offsetof", eTagHasType)

/// an uninitialized value
TREE_KIND(eTreeExprEmpty,  "expr_empty",  eTagHasType)

/// a unit value
TREE_KIND(eTreeExprUnit,   "expr_unit",   eTagHasType)

/// a boolean value
TREE_KIND(eTreeExprBool,   "expr_bool",   eTagHasType)

/// a digit value
TREE_KIND(eTreeExprDigit,  "expr_digit",  eTagHasType)

/// a string literal
TREE_KIND(eTreeExprString, "expr_string", eTagHasType)

/// a cast expression
/// exact rules vary by which CAST_OP is used
TREE_KIND(eTreeExprCast,    "expr_cast",    eTagHasType)

/// load from an address
/// pointers, arrays, and references can be loaded
/// but must have a complete type, ie not eTreeTypeOpaque or eTreePartial
TREE_KIND(eTreeExprLoad,    "expr_load",    eTagHasType)

/// take the address of an object with storage
TREE_KIND(eTreeExprAddressOf, "expr_addressof", eTagHasType)

/// unary operations
TREE_KIND(eTreeExprUnary,   "expr_unary",   eTagHasType)

/// binary operations
TREE_KIND(eTreeExprBinary,  "expr_binary",  eTagHasType)

/// compare operations
TREE_KIND(eTreeExprCompare, "expr_compare", eTagHasType)

/// get a reference to a field in a struct
/// equivilent to `obj.field` in C
TREE_KIND(eTreeExprField,  "expr_field",  eTagHasType)

/// get a reference to an index in an array
/// equivalent to `arr[idx]` in C (but arr must be a pointer type)
TREE_KIND(eTreeExprOffset, "expr_offset", eTagHasType)

/// call a function
/// supports both direct and indirect calls
TREE_KIND(eTreeExprCall, "expr_call", eTagHasType)

///
/// statements
/// these cannot yield a value
///

/// a dummy statement node to be used when passing builtins/attribute statements to
/// a backend
TREE_KIND(eTreeStmtBuiltin, "stmt_builtin", eTagNone)
TREE_KIND(eTreeStmtBlock,   "stmt_block",  eTagNone)
TREE_KIND(eTreeStmtReturn,  "stmt_return", eTagNone)
TREE_KIND(eTreeStmtAssign,  "stmt_assign", eTagNone)
TREE_KIND(eTreeStmtLoop,    "stmt_loop",   eTagNone)
TREE_KIND(eTreeStmtBranch,  "stmt_branch", eTagNone)
TREE_KIND(eTreeStmtJump,    "stmt_jump",   eTagNone) ///< loop related jumps

///
/// declarations
///

TREE_KIND(eTreeDeclAttrib,   "decl_attrib",   eTagName)
TREE_KIND(eTreeDeclGlobal,   "decl_global",   eTagName | eTagStorage | eTagEval | eTagHasType)
TREE_KIND(eTreeDeclLocal,    "decl_local",    eTagName | eTagStorage | eTagEval | eTagHasType)
TREE_KIND(eTreeDeclParam,    "decl_param",    eTagName | eTagHasType)
TREE_KIND(eTreeDeclCase,     "decl_case",     eTagName | eTagHasType)
TREE_KIND(eTreeDeclField,    "decl_field",    eTagName | eTagHasType)
TREE_KIND(eTreeDeclFunction, "decl_function", eTagName | eTagEval | eTagHasType)
TREE_KIND(eTreeDeclModule,   "decl_module",   eTagName)

TREE_KIND(eTreePartial, "partial_type", eTagName | eTagIsType) ///< unresolved type
TREE_KIND(eTreeError,   "error",        eTagName | eTagHasType) ///< an error message

#undef TREE_KIND

#ifndef DIGIT_KIND
#   define DIGIT_KIND(ID, STR)
#endif

DIGIT_KIND(eDigitChar,     "char")
DIGIT_KIND(eDigitShort,    "short")
DIGIT_KIND(eDigitInt,      "int")
DIGIT_KIND(eDigitLong,     "long")
DIGIT_KIND(eDigitLongLong, "longlong")

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

DIGIT_KIND(eDigitHalf, "half")
DIGIT_KIND(eDigitFloat, "float")
DIGIT_KIND(eDigitDouble, "double")

#undef DIGIT_KIND

#ifndef SIGN_KIND
#   define SIGN_KIND(ID, STR)
#endif

SIGN_KIND(eSignDefault,  "default") ///< dealing with C interop
SIGN_KIND(eSignUnsigned, "unsigned")
SIGN_KIND(eSignSigned,   "signed")

#undef SIGN_KIND

#ifndef TYPE_QUALIFIER
#   define TYPE_QUALIFIER(ID, STR, BIT)
#endif

TYPE_QUALIFIER(eQualNone,     "none", (0 << 0)) // defer qualifiers to parent
TYPE_QUALIFIER(eQualConst,    "const", (1 << 0))
TYPE_QUALIFIER(eQualMutable,  "mutable", (1 << 1))
TYPE_QUALIFIER(eQualVolatile, "volatile", (1 << 2))
TYPE_QUALIFIER(eQualAtomic,   "atomic", (1 << 3))
TYPE_QUALIFIER(eQualRestrict, "restrict", (1 << 4))

#undef TYPE_QUALIFIER

/// symbol visibility during linking
#ifndef TREE_LINKAGE
#   define TREE_LINKAGE(ID, STR)
#endif

TREE_LINKAGE(eLinkExport,           "exported")
TREE_LINKAGE(eLinkImport,           "imported")
TREE_LINKAGE(eLinkModule,           "internal")
TREE_LINKAGE(eLinkEntryCli,         "entry_cli")
TREE_LINKAGE(eLinkEntryGui,         "entry_gui")
TREE_LINKAGE(eLinkPixelShader,      "pixel_shader")
TREE_LINKAGE(eLinkVertexShader,     "vertex_shader")
TREE_LINKAGE(eLinkComputeShader,    "compute_shader")

#undef TREE_LINKAGE

/// symbol visibility during compilation
#ifndef TREE_VISIBILITY
#    define TREE_VISIBILITY(ID, NAME)
#endif

TREE_VISIBILITY(eVisiblePublic,  "public")
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

#ifndef TREE_EVAL_MODEL
#   define TREE_EVAL_MODEL(ID, NAME, BIT)
#endif

TREE_EVAL_MODEL(eEvalRuntime, "runtime", (1 << 0)) // can be evaluated at runtime
TREE_EVAL_MODEL(eEvalCompile, "compile", (1 << 1)) // can be evaluated at compile time

#undef TREE_EVAL_MODEL
