#include "common.h"

#include "std/typevec.h"

#include "report/report.h"

static bool run_exec(reports_t *reports, ssa_module_t *mod, typevec_t *passes)
{
    bool dirty = false;
    size_t len = typevec_len(passes);

    for (size_t i = 0; i < len; i++)
    {
        ssa_pass_t *pass = typevec_offset(passes, i);
        logverbose("running pass %s", pass->name);
        dirty |= pass->run(reports, mod);
    }

    return dirty;
}

void ssa_opt(reports_t *reports, ssa_module_t *mod, typevec_t *passes)
{
    size_t run = 0;
    while (run_exec(reports, mod, passes))
    {
        logverbose("running full pass %zu", ++run);
    }
}
