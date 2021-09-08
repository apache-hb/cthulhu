#include "report2.h"

reports_t *begin_report(size_t limit) {

}

int end_report(reports_t *reports, const char *name) {

}

message_t *assertf(reports_t *reports, const char *fmt, ...);

message_t *reportf(reports_t *reports, const node_t *node, const char *fmt, ...);
message_t *reportv(reports_t *reports, const node_t *node, const char *fmt, va_list args);

void report_appendf(message_t *message, const node_t *node, const char *fmt, ...);
void report_appendv(message_t *message, const node_t *node, const char *fmt, va_list args);
void report_note(message_t *message, const char *fmt, ...);
