#include "cthulhu/hlir/hlir.h"

extern const hlir_attributes_t DEFAULT_ATTRIBS;
extern hlir_t *TYPE;
extern hlir_t *INVALID;

hlir_t *hlir_new(IN const node_t *node, 
                 IN const hlir_t *of, 
                 hlir_kind_t kind);

hlir_t *hlir_new_decl(IN const node_t *node, 
                      IN const char *name, 
                      IN const hlir_t *of, 
                      hlir_kind_t kind);

hlir_t *hlir_new_forward(IN const node_t *node, 
                         IN const char *name, 
                         IN const hlir_t *of, 
                         hlir_kind_t expect);

#define CHECK_NULL(expr) CTASSERT(expr != NULL, "null pointer")
