#include "report-ext.h"


message_t *report_shadow(reports_t *reports,
                        const char *name,
                        const node_t *shadowed,
                        const node_t *shadowing)
{
    message_t *id = report2(reports, ERROR, shadowing, "redefinition of `%s`", name);
    report_append2(id, shadowed, "previous definition");
    return id;
}
