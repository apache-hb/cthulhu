#ifndef FMT_HEADING
#   define FMT_HEADING(id, name)
#endif

/// @brief generic style
/// ie: `file:line:col:`
FMT_HEADING(eHeadingGeneric, "generic")

/// @brief microsoft msvc style
/// ie: `file(line:col):`
FMT_HEADING(eHeadingMicrosoft, "microsoft")

#undef FMT_HEADING

#ifndef FMT_BT_OPTION
#   define FMT_BT_OPTION(id, name, value)
#endif

FMT_BT_OPTION(eBtZeroIndexedLines, "zero indexed lines", (1 << 0))
FMT_BT_OPTION(eBtDemangleNames, "demangle names", (1 << 1))
FMT_BT_OPTION(eBtAlwaysShowAddress, "always show address", (1 << 2))

#undef FMT_BT_OPTION
