#ifndef C_AST_KIND
#   define C_AST_KIND(id, name)
#endif

C_AST_KIND(eAstTypedef, "typedef")
C_AST_KIND(eAstStruct, "struct")
C_AST_KIND(eAstUnion, "union")
C_AST_KIND(eAstEnum, "enum")

C_AST_KIND(eAstEnumValue, "enum_value")
C_AST_KIND(eAstTypeField, "type_field")

C_AST_KIND(eAstAlignas, "alignas")
C_AST_KIND(eAstSizeof, "sizeof")
C_AST_KIND(eAstAlignof, "alignof")
C_AST_KIND(eAstTypename, "typename")

C_AST_KIND(eAstInitDeclarator, "init_declarator")
C_AST_KIND(eAstDeclaratorList, "declarator_list")

// expressions
C_AST_KIND(eAstExprList, "expr_list")
C_AST_KIND(eAstCast, "cast")
C_AST_KIND(eAstString, "string")

// intermediate type
C_AST_KIND(eAstOpaque, "opaque")

C_AST_KIND(eAstLabel, "label")
C_AST_KIND(eAstCase, "case")
C_AST_KIND(eAstDefault, "default")

C_AST_KIND(eAstStorageClass, "storage_class")
C_AST_KIND(eAstTypeQualifier, "type_qualifier")

C_AST_KIND(eAstAttributeNoreturn, "attribute_noreturn")
C_AST_KIND(eAstAttributeCallConv, "attribute_callconv")

// module extensions
C_AST_KIND(eAstModulePrivateFragment, "module_private_fragment")
C_AST_KIND(eAstModulePublicFragment, "module_public_fragment")
C_AST_KIND(eAstModuleImport, "module_import")

#undef C_AST_KIND

#ifndef C_STORAGE_CLASS
#   define C_STORAGE_CLASS(id, name, v)
#endif

C_STORAGE_CLASS(eStorageClassAuto, "auto", (1 << 0))
C_STORAGE_CLASS(eStorageClassRegister, "register", (1 << 1))
C_STORAGE_CLASS(eStorageClassStatic, "static", (1 << 2))
C_STORAGE_CLASS(eStorageClassExtern, "extern", (1 << 3))
C_STORAGE_CLASS(eStorageClassTypedef, "typedef", (1 << 4))
C_STORAGE_CLASS(eStorageClassThreadLocal, "thread_local", (1 << 5))
C_STORAGE_CLASS(eStorageClassInline, "inline", (1 << 6))
C_STORAGE_CLASS(eStorageClassConstexpr, "constexpr", (1 << 7))

#undef C_STORAGE_CLASS

#ifndef C_TYPE_QUALIFIER
#   define C_TYPE_QUALIFIER(id, name, v)
#endif

C_TYPE_QUALIFIER(eTypeQualifierConst, "const", (1 << 0))
C_TYPE_QUALIFIER(eTypeQualifierVolatile, "volatile", (1 << 1))
C_TYPE_QUALIFIER(eTypeQualifierRestrict, "restrict", (1 << 2))
C_TYPE_QUALIFIER(eTypeQualifierAtomic, "atomic", (1 << 3))

#undef C_TYPE_QUALIFIER

#ifndef C_TYPE_SPECIFIER
#   define C_TYPE_SPECIFIER(id, name)
#endif

C_TYPE_SPECIFIER(eTypeSpecifierVoid, "void")
C_TYPE_SPECIFIER(eTypeSpecifierChar, "char")
C_TYPE_SPECIFIER(eTypeSpecifierShort, "short")
C_TYPE_SPECIFIER(eTypeSpecifierInt, "int")
C_TYPE_SPECIFIER(eTypeSpecifierLong, "long")
C_TYPE_SPECIFIER(eTypeSpecifierFloat, "float")
C_TYPE_SPECIFIER(eTypeSpecifierDouble, "double")
C_TYPE_SPECIFIER(eTypeSpecifierSigned, "signed")
C_TYPE_SPECIFIER(eTypeSpecifierUnsigned, "unsigned")
C_TYPE_SPECIFIER(eTypeSpecifierBool, "bool")
C_TYPE_SPECIFIER(eTypeSpecifierComplex, "complex")

#undef C_TYPE_SPECIFIER

#ifndef C_CALLCONV
#   define C_CALLCONV(id, name)
#endif

C_CALLCONV(eCallCdecl, "cdecl")
C_CALLCONV(eCallStdcall, "stdcall")
C_CALLCONV(eCallFastcall, "fastcall")
C_CALLCONV(eCallThiscall, "thiscall")
C_CALLCONV(eCallVectorcall, "vectorcall")

#undef C_CALLCONV
