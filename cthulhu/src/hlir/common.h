#include "cthulhu/hlir/hlir.h"

#include "base/macros.h"
#include "report/report.h"

extern hlir_t *kMetaType;
extern hlir_t *kInvalidNode;

hlir_t *hlir_new(node_t node, const hlir_t *of, hlir_kind_t kind);

hlir_t *hlir_new_decl(node_t node, const char *name, const hlir_t *of, hlir_kind_t kind);

hlir_t *hlir_new_forward(node_t node, const char *name, const hlir_t *of, hlir_kind_t expect);

node_t check_const_expr(reports_t *reports, const hlir_t *expr);
