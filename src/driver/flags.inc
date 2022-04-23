#ifndef DEFAULT_REPORT_LIMIT
#   define DEFAULT_REPORT_LIMIT 20
#endif

#ifndef COMMAND
#   define COMMAND(id, type, initial, description, ...)
#endif

#ifndef SECTION
#   define SECTION(id)
#endif

SECTION("general")

COMMAND(printHelp, bool, false, "print this help message", { "--help", "-h", "/?" })
COMMAND(printVersion, bool, false, "print the version number", { "--version", "-v" })

SECTION("reporting")

COMMAND(verboseLogging, bool, false, "print verbose information", { "-Wverbose" })
COMMAND(warningLimit, int, DEFAULT_REPORT_LIMIT, "limit the number of reported errors", { "--Wlimit" })

SECTION("codegen")

COMMAND(enableBytecode, bool, false, "enable bytecode generation", { "-B" })
COMMAND(embedSource, bool, false, "embed the source code in the bytecode", { "-Bembed" })
COMMAND(moduleName, const char *, NULL, "override default module name", { "-Bmodule" })
COMMAND(outputFile, const char *, NULL, "override default output file", { "-O" })

#undef COMMAND
#undef SECTION
