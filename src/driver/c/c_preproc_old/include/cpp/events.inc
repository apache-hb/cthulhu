#ifndef NEW_EVENT
#   define NEW_EVENT(id, ...)
#endif

NEW_EVENT(IncludeDepthExceeded, {
    .severity = eSeverityFatal,
    .id = "PP0001",
    .brief = "Include depth exceeded",
    .description =
        "The maximum include depth has been exceeded. This limit can be increased for complex include heirarchies\n"
        "Alternatively this may be a sign of a circular include",
})

NEW_EVENT(IncludeNotFound, {
    .severity = eSeverityFatal,
    .id = "PP0002",
    .brief = "Include not found",
    .description =
        "The include file could not be found. Check that the path is spelt correctly and\n"
        "the correct include directories have been provided.",
})

NEW_EVENT(UnexpectedElse, {
    .severity = eSeverityWarn,
    .id = "PP0003",
    .brief = "Unexpected else directive",
    .description =
        "There are an unbalanced number of `#else` directives. This may be a sign of a missing `#if/#ifdef` or `#endif`",
})

NEW_EVENT(UnexpectedEndIf, {
    .severity = eSeverityWarn,
    .id = "PP0003",
    .brief = "Too many endif directives",
    .description =
        "There are an unbalanced number of `#endif` directives. This may be a sign of a missing `#if/#ifdef` or `#else/#elif`",
})

NEW_EVENT(MacroRedefinition, {
    .severity = eSeverityWarn,
    .id = "PP0004",
    .brief = "Macro redefinition",
    .description =
        "A macro has been redefined without first being undefined.",
})

NEW_EVENT(MacroNotDefined, {
    .severity = eSeverityWarn,
    .id = "PP0005",
    .brief = "Macro not defined",
    .description =
        "Attempting to undefine a macro that was never defined.",
})

NEW_EVENT(UnexpectedElif, {
    .severity = eSeverityWarn,
    .id = "PP0006",
    .brief = "Unexpected elif directive",
    .description =
        "There are an unbalanced number of `#elif` directives. This may be a sign of a missing `#if/#ifdef` or `#else`",
})

NEW_EVENT(UnexpectedToken, {
    .severity = eSeverityFatal,
    .id = "PP0007",
    .brief = "Unexpected token during macro evaluation",
    .description =
        "Malformed token encountered during macro evaluation",
})

NEW_EVENT(UnknownPragma, {
    .severity = eSeverityWarn,
    .id = "PP0008",
    .brief = "Unknown pragma",
    .description =
        "Unknown pragma directive encountered",
})

NEW_EVENT(TooManyArgs, {
    .severity = eSeverityWarn,
    .id = "PP0009",
    .brief = "Too many arguments",
    .description =
        "Too many arguments provided to a macro",
})

#undef NEW_EVENT
