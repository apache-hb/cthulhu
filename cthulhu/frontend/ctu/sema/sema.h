#pragma once

#include "cthulhu/driver/cmd.h"
#include "ast.h"
#include "cthulhu/lir/sema.h"

lir_t *ctu_sema(reports_t *reports, ctu_t *ctu);
sema_t *ctu_start(reports_t *reports, ctu_t *ctu);
lir_t *ctu_finish(sema_t *sema);

vector_t *ctu_analyze(reports_t *reports, settings_t *settings, ctu_t *ctu);

lir_t *ctu_forward(node_t *node, const char *name, leaf_t leaf, void *data);
