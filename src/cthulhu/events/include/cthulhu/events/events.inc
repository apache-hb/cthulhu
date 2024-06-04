// SPDX-License-Identifier: LGPL-3.0-only

#ifndef CTU_EVENT
#   define CTU_EVENT(name, ...)
#endif

/// vocabulary
/// compiler: the current instance of the mediator
/// translation unit: a single compilation unit, a single source file can have multiple translation units

CTU_EVENT(NoSourceFiles, {
    .severity = eSeverityFatal,
    .id = "M0001",
    .brief = "No source files",
    .description = "No source files were provided to the compiler",
})

CTU_EVENT(FailedToOpenSourceFile, {
    .severity = eSeverityFatal,
    .id = "M0002",
    .brief = "Failed to open source file",
    .description = "Failed to open a required source file required by the compiler",
})

CTU_EVENT(NoFileExtension, {
    .severity = eSeverityFatal,
    .id = "M0003",
    .brief = "No file extension",
    .description = "The compiler was unable to identify the language of a source file due to the"
                   " lack of a file extension",
})

CTU_EVENT(FailedToCreateOutputDirectory, {
    .severity = eSeverityFatal,
    .id = "M0004",
    .brief = "Failed to create output directory",
    .description = "The compiler was unable to create the output directory",
})

CTU_EVENT(FailedToCreateOutputFile, {
    .severity = eSeverityFatal,
    .id = "M0005",
    .brief = "Failed to create output file",
    .description = "The compiler was unable to create the output file",
})

CTU_EVENT(FailedToWriteOutputFile, {
    .severity = eSeverityFatal,
    .id = "M0006",
    .brief = "Failed to write output file",
    .description = "The compiler was unable to write to the output file",
})

CTU_EVENT(ExtensionConflict, {
    .severity = eSeverityInternal,
    .id = "M0007",
    .brief = "Extension conflict",
    .description = "Multiple languages enabled in the compiler register the same file extension",
})

CTU_EVENT(ParseInitFailed, {
    .severity = eSeverityInternal,
    .id = "M0008",
    .brief = "Failed to init parser",
    .description = "The parser failed to initialise",
})

CTU_EVENT(ScanFailed, {
    .severity = eSeverityInternal,
    .id = "M0009",
    .brief = "Failed to scan",
    .description = "The scanner failed to scan the source file due to internal errors",
})

CTU_EVENT(ParseFailed, {
    .severity = eSeverityFatal,
    .id = "M0010",
    .brief = "Failed to parse",
    .description = "The parser failed to parse the source file",
})

CTU_EVENT(ParseInternalError, {
    .severity = eSeverityInternal,
    .id = "M0011",
    .brief = "Internal parser error",
    .description = "The parser encountered an internal error",
})

CTU_EVENT(UninitializedValueUsed, {
    .severity = eSeverityFatal,
    .id = "M0012",
    .brief = "Uninitialized value used",
    .description = "An uninitialized value was used in a compile time context",
})

CTU_EVENT(DivideByZero, {
    .severity = eSeverityFatal,
    .id = "M0013",
    .brief = "Divide by zero",
    .description = "A divide by zero was encountered in a compile time context",
})

CTU_EVENT(ModuloByZero, {
    .severity = eSeverityFatal,
    .id = "M0014",
    .brief = "Modulo by zero",
    .description = "A modulo by zero was encountered in a compile time context",
})

CTU_EVENT(InvalidStringEscape, {
    .severity = eSeverityFatal,
    .id = "M0015",
    .brief = "Invalid string escape",
    .description = "An invalid string escape was encountered while parsing",
})

CTU_EVENT(InvalidIntegerLiteral, {
    .severity = eSeverityFatal,
    .id = "M0016",
    .brief = "Invalid integer literal",
    .description = "An invalid integer literal was encountered while parsing",
})

CTU_EVENT(SymbolShadowed, {
    .severity = eSeverityFatal,
    .id = "M0017",
    .brief = "Symbol shadowed",
    .description = "A symbol was shadowed by another symbol",
})

CTU_EVENT(VariableNotFound, {
    .severity = eSeverityFatal,
    .id = "M0018",
    .brief = "Variable not found",
    .description = "A variable was not found in the current scope",
})

CTU_EVENT(FunctionNotFound, {
    .severity = eSeverityFatal,
    .id = "M0019",
    .brief = "Function not found",
    .description = "A function was not found in the current scope",
})

CTU_EVENT(TypeNotFound, {
    .severity = eSeverityFatal,
    .id = "M0020",
    .brief = "Type not found",
    .description = "A type was not found in the current scope",
})

CTU_EVENT(FieldNotFound, {
    .severity = eSeverityFatal,
    .id = "M0021",
    .brief = "Field not found",
    .description = "A field was not found in within a type",
})

CTU_EVENT(AttribNotFound, {
    .severity = eSeverityWarn,
    .id = "M0022",
    .brief = "Attrib not found",
    .description = "An attribute of the current name was not found, "
    "or is not supported by this version of the compiler",
})

CTU_EVENT(InvalidIndirection, {
    .severity = eSeverityFatal,
    .id = "M0023",
    .brief = "Invalid indirection",
    .description = "The wrong indirection was used when accessing a value",
})

CTU_EVENT(InvalidIndex, {
    .severity = eSeverityFatal,
    .id = "M0024",
    .brief = "Invalid index",
    .description = "An indexing operation was invalid",
})

CTU_EVENT(SymbolNotVisible, {
    .severity = eSeverityFatal,
    .id = "M0025",
    .brief = "Symbol not visible",
    .description = "A symbol was not visible in the current scope, perhaps due to privacy specifiers",
})

CTU_EVENT(SymbolNotFound, {
    .severity = eSeverityFatal,
    .id = "M0026",
    .brief = "Symbol not found",
    .description = "A symbol was not found in the current scope",
})

CTU_EVENT(MalformedTypeName, {
    .severity = eSeverityFatal,
    .id = "M0027",
    .brief = "Malformed type name",
    .description = "A type name was malformed and could not be resolved to a type",
})

CTU_EVENT(MutationOfConst, {
    .severity = eSeverityFatal,
    .id = "M0028",
    .brief = "Mutation of const",
    .description = "A const value was mutated",
})

CTU_EVENT(ExpressionHasNoEffect, {
    .severity = eSeverityWarn,
    .id = "M0029",
    .brief = "Expression has no effect",
    .description = "An expression has no effect",
})

CTU_EVENT(IncorrectParamCount, {
    .severity = eSeverityFatal,
    .id = "M0030",
    .brief = "Incorrect parameter count",
    .description = "A function was called with an incorrect number of parameters",
})

CTU_EVENT(IncorrectParamType, {
    .severity = eSeverityFatal,
    .id = "M0031",
    .brief = "Incorrect parameter type",
    .description = "A function was called with an incorrect parameter type",
})

CTU_EVENT(DuplicateAttribute, {
    .severity = eSeverityFatal,
    .id = "M0032",
    .brief = "Duplicate attribute",
    .description = "A duplicate attribute was found on a declaration",
})

CTU_EVENT(InvalidAttributeApplication, {
    .severity = eSeverityFatal,
    .id = "M0033",
    .brief = "Invalid attribute application",
    .description = "An attribute was applied to the incorrect kind of declaration",
})

CTU_EVENT(MalformedAttribute, {
    .severity = eSeverityFatal,
    .id = "M0034",
    .brief = "Malformed attribute",
    .description = "An attribute or its arguments were malformed",
})

CTU_EVENT(UnimplementedAttribute, {
    .severity = eSeverityWarn,
    .id = "M0035",
    .brief = "Unimplemented attribute",
    .description = "An attribute is not implemented",
})

CTU_EVENT(InvalidLiteralType, {
    .severity = eSeverityFatal,
    .id = "M0036",
    .brief = "Invalid literal type",
    .description = "A literal was of an invalid type",
})

CTU_EVENT(InvalidBinaryOperation, {
    .severity = eSeverityFatal,
    .id = "M0037",
    .brief = "Invalid binary operation",
    .description = "A binary operations operands were not compatible for the operation",
})

CTU_EVENT(InvalidUnaryOperation, {
    .severity = eSeverityFatal,
    .id = "M0038",
    .brief = "Invalid unary operation",
    .description = "A unary operations operand was not compatible for the operation",
})

CTU_EVENT(InvalidInitializer, {
    .severity = eSeverityFatal,
    .id = "M0039",
    .brief = "Invalid initializer",
    .description = "An aggregate initializer was invalid,"
    " either due to an invalid field value or an ambiguous implicit type",
})

CTU_EVENT(InvalidControlFlow, {
    .severity = eSeverityFatal,
    .id = "M0040",
    .brief = "Invalid control flow",
    .description = "A control flow statement was nonsensical in the current context."
    " a break or continue outside of a loop or a goto with a non-local label.",
})

CTU_EVENT(CirclularImport, {
    .severity = eSeverityFatal,
    .id = "M0041",
    .brief = "Circular import",
    .description = "A translation unit imported itself, note that multural recursion between translation units is supported",
})

CTU_EVENT(ImportNotFound, {
    .severity = eSeverityFatal,
    .id = "M0042",
    .brief = "Import not found",
    .description = "An import was unable to be resolved",
})

CTU_EVENT(ImportShadowed, {
    .severity = eSeverityFatal,
    .id = "M0043",
    .brief = "Import shadowed",
    .description = "Importing an external translation units symbols would shadow an existing symbol in the current scope",
})

CTU_EVENT(InvalidEnumUnderlyingType, {
    .severity = eSeverityFatal,
    .id = "M0044",
    .brief = "Invalid enum underlying type",
    .description = "The user specified underlying type of an enum was invalid",
})

CTU_EVENT(DuplicateField, {
    .severity = eSeverityFatal,
    .id = "M0045",
    .brief = "Duplicate field",
    .description = "A duplicate field was found in an aggregate or enumeration",
})

CTU_EVENT(InvalidFunctionSignature, {
    .severity = eSeverityFatal,
    .id = "M0046",
    .brief = "Invalid function signature",
    .description = "A function signature was invalid, parameters cannot be a unit type",
})

CTU_EVENT(InvalidArraySize, {
    .severity = eSeverityFatal,
    .id = "M0047",
    .brief = "Invalid array size",
    .description = "An array size was invalid, either due to a non-integer size or a negative size,"
    " the size of an array must be statically known at compile time",
})

CTU_EVENT(InvalidVariableType, {
    .severity = eSeverityFatal,
    .id = "M0048",
    .brief = "Invalid variable type",
    .description = "A variable was declared with an invalid type,"
    " either a unit type or a type with an invalid storage class",
})

CTU_EVENT(CyclicDependency, {
    .severity = eSeverityFatal,
    .id = "M0049",
    .brief = "Cyclic dependency",
    .description = "Static evaluation of a value encountered a cyclic dependency",
})

CTU_EVENT(ImportedWithImpl, {
    .severity = eSeverityFatal,
    .id = "M0050",
    .brief = "Imported with impl",
    .description = "A declaration was imported but has a declaration with an implementation in the current translation unit",
})

CTU_EVENT(EntryNotFunction, {
    .severity = eSeverityFatal,
    .id = "M0051",
    .brief = "Entry not function",
    .description = "The entry point of the program was not a function",
})

CTU_EVENT(EntryMissingBody, {
    .severity = eSeverityFatal,
    .id = "M0052",
    .brief = "Entry missing body",
    .description = "The entry point of the program was missing an implementation",
})

CTU_EVENT(MultipleEntryPoints, {
    .severity = eSeverityFatal,
    .id = "M0053",
    .brief = "Multiple entry points",
    .description = "A program was declared with multiple entry points of the same type"
    " a program can only have one entry point of each type",
})

CTU_EVENT(ReturnTypeMismatch, {
    .severity = eSeverityFatal,
    .id = "M0054",
    .brief = "Return type mismatch",
    .description = "A function returned a value of a different type than its return type"
    " a return statement must return a value that can be implicitly cast to the functions declared return type",
})

CTU_EVENT(Deprecated, {
    .severity = eSeverityWarn,
    .id = "M0055",
    .brief = "Deprecated",
    .description = "A deprecated feature was used",
})

CTU_EVENT(MayNotReturn, {
    .severity = eSeverityFatal,
    .id = "M0056",
    .brief = "May not return",
    .description = "A function with a return value may not always return a value",
})

CTU_EVENT(RecursiveEval, {
    .severity = eSeverityFatal,
    .id = "M0057",
    .brief = "Recursive eval",
    .description = "Static evaluation of a value encountered a infinite recursion",
})

CTU_EVENT(InfiniteSizedType, {
    .severity = eSeverityFatal,
    .id = "M0058",
    .brief = "Infinite sized type",
    .description = "A type was infinitely sized, indicating a recursive type with no indirection",
})

CTU_EVENT(InvalidType, {
    .severity = eSeverityFatal,
    .id = "M0059",
    .brief = "Invalid type",
    .description = "A type was invalid due to invalid recursion",
})

CTU_EVENT(WorryingCast, {
    .severity = eSeverityWarn,
    .id = "M0061",
    .brief = "Worrying cast",
    .description = "A cast between types that may truncate, lose precision, or lose type information",
})

CTU_EVENT(InvalidCast, {
    .severity = eSeverityFatal,
    .id = "M0060",
    .brief = "Invalid cast",
    .description = "A cast between types was invalid",
})

CTU_EVENT(FailedToIdentifyLanguage, {
    .severity = eSeverityFatal,
    .id = "M0062",
    .brief = "Failed to identify language",
    .description = "The compiler was unable to identify the language of a source file",
})

CTU_EVENT(IgnoredMangling, {
    .severity = eSeverityWarn,
    .id = "M0063",
    .brief = "Ignored mangling",
    .description = "A symbol with hidden linkage has a mangled name, which is ignored",
})

CTU_EVENT(UnknownToken, {
    .severity = eSeverityFatal,
    .id = "M0064",
    .brief = "Unknown token",
    .description = "The scanner encountered an unknown token",
})

CTU_EVENT(SyntaxError, {
    .severity = eSeverityFatal,
    .id = "M0065",
    .brief = "Syntax error",
    .description = "The parser encountered a syntax error",
})

CTU_EVENT(SourceAndHeaderOutput, {
    .severity = eSeverityFatal,
    .id = "M0066",
    .brief = "Source and header both need to be specified",
    .description =
        "When generating C output files it is invalid to specify only "
        "a source or header file, either both or neither must be specified",
})

CTU_EVENT(ExactlyOneSourceFile, {
    .severity = eSeverityFatal,
    .id = "M0067",
    .brief = "Tool expects exactly one source file",
    .description =
        "The tool expects exactly one source file, but multiple were provided",
})

CTU_EVENT(CannotEvalExpression, {
    .severity = eSeverityWarn,
    .id = "M0068",
    .brief = "Evaluation of an expression failed",
    .description =
        "The compiler was unable to evaluate an expression, "
        "this may be due to an unsupported feature or a bug in the compiler.",
})

CTU_EVENT(InexactIntegerDivision, {
    .severity = eSeverityWarn,
    .id = "M0069",
    .brief = "Inexact integer division",
    .description =
        "An integer division was not exact, "
        "This behaviour is not consistent between languages or compiler versions.",
})

CTU_EVENT(ShiftByNegative, {
    .severity = eSeverityFatal,
    .id = "M0070",
    .brief = "Shift by negative",
    .description =
        "A bit shift by a negative amount was encountered.",
})

CTU_EVENT(IntegerOverflow, {
    .severity = eSeverityFatal,
    .id = "M0071",
    .brief = "Integer overflow",
    .description =
        "An integer overflow was encountered.",
})

CTU_EVENT(InvalidFloatLiteral, {
    .severity = eSeverityFatal,
    .id = "M0072",
    .brief = "Invalid float literal",
    .description =
        "Invalid floating point literal value.",
})

CTU_EVENT(InvalidFilePath, {
    .severity = eSeverityFatal,
    .id = "M0073",
    .brief = "Invalid file path",
    .description =
        "The file path was invalid.",
})

CTU_EVENT(ModuleConflict, {
    .severity = eSeverityFatal,
    .id = "M0074",
    .brief = "Module conflict",
    .description =
        "A module with the same name already exists.",
})

CTU_EVENT(LanguageDriverConflict, {
    .severity = eSeverityFatal,
    .id = "M0075",
    .brief = "Language driver conflict",
    .description =
        "A language driver with the same name already exists.",
})

CTU_EVENT(PluginConflict, {
    .severity = eSeverityFatal,
    .id = "M0076",
    .brief = "Plugin conflict",
    .description =
        "A plugin with the same name already exists.",
})

CTU_EVENT(TargetConflict, {
    .severity = eSeverityFatal,
    .id = "M0077",
    .brief = "Target conflict",
    .description =
        "A target with the same name already exists.",
})

CTU_EVENT(DynamicLoadingDisabled, {
    .severity = eSeverityFatal,
    .id = "M0078",
    .brief = "Dynamic loading disabled",
    .description =
        "Dynamic loading is disabled in this distribution of cthulhu.",
})

CTU_EVENT(Unimplemented, {
    .severity = eSeveritySorry,
    .id = "M0079",
    .brief = "Unimplemented",
    .description =
        "This feature has not yet been implemented.",
})

CTU_EVENT(CommandLineInvalid, {
    .severity = eSeverityFatal,
    .id = "M0080",
    .brief = "Command line invalid",
    .description =
        "The command line arguments were invalid.",
})

CTU_EVENT(InvalidTarget, {
    .severity = eSeverityFatal,
    .id = "M0081",
    .brief = "Invalid target",
    .description =
        "The target was invalid.",
})

CTU_EVENT(InvalidName, {
    .severity = eSeverityInternal,
    .id = "M0082",
    .brief = "Invalid name",
    .description =
        "Non-anonymous symbol had an empty name.",
})

#undef CTU_EVENT
