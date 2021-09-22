#include "c89.h"

#include "ctu/util/str.h"

typedef struct {
    stream_t *globals;
    stream_t *functions;
} context_t;

bool c89_build(reports_t *reports, module_t *mod, const char *path) {
    assert2(reports, "c89 unimplemented %p %s", mod, path);
    return false;
}
