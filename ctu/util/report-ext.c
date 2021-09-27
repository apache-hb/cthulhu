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

message_t *report_recursive(reports_t *reports,
                            vector_t *stack,
                            lir_t *root)
{
    node_t *node = root->node;
    message_t *id = report2(reports, ERROR, node, "initialization of `%s` is recursive", root->name);
    
    node_t *last = node;
    size_t len = vector_len(stack);
    size_t t = 0;

    for (size_t i = 0; i < len; i++) {
        node_t *it = vector_get(stack, i);
        if (it != last) {
            report_append2(id, it, "trace %zu", t++);
        }
        last = it;
    }

    return id;
}
