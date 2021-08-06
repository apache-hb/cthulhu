#include "speed.h"

#include "ctu/util/util.h"
#include "ctu/util/report.h"
#include "ctu/debug/ir.h"

#define FINISHED_OPTIMIZATION SIZE_MAX

pass_t new_pass(module_t *mod) {
    size_t len = num_flows(mod);
    pass_t pass = { mod, ctu_malloc(sizeof(size_t) * len) };

    for (size_t i = 0; i < len; i++) {
        pass.dirty[i] = 0;
    }

    return pass;
}

static bool is_dirty(pass_t *pass, size_t idx) {
    return pass->dirty[idx] != FINISHED_OPTIMIZATION;
}

static void end_pass(pass_t *pass) {
    for (size_t i = 0; i < num_flows(pass->mod); i++) {
        if (pass->dirty[i] == 0) {
            pass->dirty[i] = FINISHED_OPTIMIZATION;
        } else if (is_dirty(pass, i)) {
            pass->dirty[i] = 0;
        }
    }
}

static void mark_dirty(pass_t *pass, size_t idx) {
    pass->dirty[idx] += 1;
}

bool run_pass(pass_t *pass) {
    bool changed = false;
    for (size_t i = 0; i < num_flows(pass->mod); i++) {
        if (!is_dirty(pass, i)) {
            continue;
        }

        flow_t *flow = pass->mod->flows + i;

        logfmt("begin pass for %s", flow_name(flow));

        if (remove_unused_blocks(flow)) {
            logfmt("removed unused blocks");
            mark_dirty(pass, i);
        }

        if (mem2reg(flow)) {
            logfmt("reduced memory");
            mark_dirty(pass, i);
        }

        if (propogate_consts(flow)) {
            logfmt("propogated values");
            mark_dirty(pass, i);
        }

        if (remove_unused_code(flow)) {
            logfmt("removed unreferenced vregs");
            mark_dirty(pass, i);
        }

        if (remove_empty_blocks(flow)) {
            logfmt("removed empty blocks");
            mark_dirty(pass, i);
        }

        if (remove_branches(flow)) {
            logfmt("removed excess branches");
            mark_dirty(pass, i);
        }

        if (remove_jumps(flow)) {
            logfmt("removed excess jumps");
            mark_dirty(pass, i);
        }

        if (remove_pure_code(flow)) {
            logfmt("removed unused pure operations");
            mark_dirty(pass, i);
        }

        if (fold_consts(flow)) {
            logfmt("folded constant expressions");
            mark_dirty(pass, i);
        }

        if (remove_casts(flow)) {
            logfmt("removed duplicate casts");
            mark_dirty(pass, i);
        }

        if (is_dirty(pass, i)) {
            changed = true;
        }
    }

    end_pass(pass);
    return changed;
}
