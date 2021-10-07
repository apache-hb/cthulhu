#include "attrib.h"

static void compile_attrib(reports_t *reports, attrib_t *dst, ctu_t *attrib) {
    const char *name = attrib->name;
    if (streq(name, "entry")) {
        dst->visibility = ENTRYPOINT;
        dst->mangle = "main";
    } else {
        report(reports, WARNING, attrib->node, "unknown attribute `%s`", name);
    }
}

void compile_attribs(reports_t *reports, lir_t *lir, ctu_t *ctu) {
    vector_t *attribs = ctu->attribs;

    attrib_t *attr = ctu_malloc(sizeof(attrib_t));
    attr->visibility = ctu->exported ? PUBLIC : PRIVATE;
    attr->mangle = NULL;

    for (size_t i = 0; i < vector_len(attribs); i++) {
        compile_attrib(reports, attr, vector_get(attribs, i));
    }

    lir_attribs(lir, attr);
}
