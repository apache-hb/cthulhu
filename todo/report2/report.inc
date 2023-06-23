#ifndef REPORT_LEVEL
#   define REPORT_LEVEL(ID, STR, COLOUR)
#endif

REPORT_LEVEL(eInfo, "info", COLOUR_GREEN)
REPORT_LEVEL(eWarning, "warning", COLOUR_YELLOW)
REPORT_LEVEL(eFatal, "fatal", COLOUR_RED)
REPORT_LEVEL(eInternal, "internal", COLOUR_CYAN)
REPORT_LEVEL(eSorry, "sorry", COLOUR_PURPLE)

#undef REPORT_LEVEL
