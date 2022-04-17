#include "cthulhu/hlir/hlir.h"

extern const hlir_attributes_t DEFAULT_ATTRIBS;
extern hlir_t *TYPE;
extern hlir_t *INVALID;

hlir_t *hlir_new(const node_t *node, 
                 const hlir_t *of, 
                 hlir_kind_t kind);

hlir_t *hlir_new_decl(const node_t *node, 
                      const char *name, 
                      const hlir_t *of, 
                      hlir_kind_t kind);

hlir_t *hlir_new_forward(const node_t *node, 
                         const char *name, 
                         const hlir_t *of, 
                         hlir_kind_t expect);

#define CHECK_NULL(expr) CTASSERT(expr != NULL, "null pointer")
