#ifndef FLAG
#   define FLAG(group, name, type, desc, ...)
#endif

#ifndef GROUP
#   define GROUP(name, desc)
#endif

GROUP(GENERAL, "general options")
GROUP(WARN, "warning options")
GROUP(OUTPUT, "output options")

FLAG(GENERAL, PRINT_HELP_MESSAGE, POSITIONAL, "print this message", "-h", "--help", "-?")
FLAG(GENERAL, PRINT_VERSION_INFO, POSITIONAL, "print version information", "-v", "--version")
FLAG(GENERAL, ENABLE_VERBOSE_LOGGING, POSITIONAL, "enable verbose logging", "-V", "--verbose")
FLAG(GENERAL, MODULE_NAME, STRING, "set module name", "-m", "--module")
FLAG(GENERAL, SEARCH_PATH, STRING, "add search path", "-I", "--include")

FLAG(WARN, DISABLE_WARNINGS, POSITIONAL, "disable all warnings", "-w0", "--no-warnings")
FLAG(WARN, WARNING_LIMIT, INT, "set warning limit", "-Wlimit")
FLAG(WARN, WARNING_AS_ERROR, POSITIONAL, "warnings as errors", "-Werror")

FLAG(OUTPUT, OUTPUT_FORMAT, STRING, "set output format", "-f", "--format")
FLAG(OUTPUT, OUTPUT_PATH, STRING, "set output path", "-o", "--output")
FLAG(OUTPUT, TARGET_ARCH, STRING, "set target architecture", "-a", "--arch")

#undef GROUP
#undef FLAG
