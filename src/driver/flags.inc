#ifndef DEFAULT_REPORT_LIMIT
#   define DEFAULT_REPORT_LIMIT 20
#endif

#ifndef COMMAND
#   define COMMAND(id, type, initial, description, ...)
#endif

#ifndef SECTION
#   define SECTION(id)
#endif

#ifndef TYPE_BOOL
#   define TYPE_BOOL
#endif

#ifndef TYPE_STRING
#   define TYPE_STRING
#endif

#ifndef TYPE_INT
#   define TYPE_INT
#endif

#ifndef TYPE_VECTOR
#   define TYPE_VECTOR
#endif

SECTION("general")

COMMAND(printHelp, TYPE_BOOL, false, "print this help message", { "--help", "-h" })
COMMAND(printVersion, TYPE_BOOL, false, "print the version number", { "--version", "-v" })

SECTION("reporting")

COMMAND(verboseLogging, TYPE_BOOL, false, "print verbose information", { "-Wverbose" })
COMMAND(warningsAsErrors, TYPE_BOOL, false, "warnings are treated as errors", { "-Werror" })
COMMAND(warningLimit, TYPE_INT, DEFAULT_REPORT_LIMIT, "limit the number of reported errors", { "-Wlimit" })

SECTION("codegen")

COMMAND(enableBytecode, TYPE_BOOL, false, "enable bytecode generation", { "-Benable" })
COMMAND(embedSource, TYPE_BOOL, false, "embed the source code in the bytecode", { "-Bembed" })
COMMAND(moduleName, TYPE_STRING, NULL, "override default module name", { "-Bmodule" })
COMMAND(outputFile, TYPE_STRING, NULL, "override default output file name", { "-o" })
COMMAND(outputTarget, TYPE_STRING, "c89", "override default output target", { "-T" })

#undef COMMAND
#undef SECTION

#undef TYPE_BOOL
#undef TYPE_STRING
#undef TYPE_INT
#undef TYPE_VECTOR
