#include "report-new/report.h"

#include "std/vector.h"
#include "std/typed/vector.h"

typedef struct segment_t {
    const node_t *node;
    char *message;
} segment_t;

typedef struct message_t {
    const report_t *base;

    const node_t *node; ///< the root location of the error

    char *message; ///< custom error message
    char *underline; ///< optional message to put next to the underline

    typevec_t *segments; ///< typevec<segment_t>
    vector_t *notes; ///< vector<const char*>
} message_t;

typedef struct reports_t {
    vector_t *types;

    vector_t *messages;
} reports_t;

typedef void (*sink_accept_t)(sink_t *self, reports_t *reports, message_t *message);

typedef struct sink_t {
    const char *name;
} sink_t;
