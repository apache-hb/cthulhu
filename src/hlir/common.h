#include "cthulhu/hlir/hlir.h"

extern const hlir_attributes_t DEFAULT_ATTRIBS;

hlir_t *hlir_new(IN const node_t *node, 
                 IN const hlir_t *of, 
                 hlir_type_t type);

hlir_t *hlir_new_decl(IN const node_t *node, 
                      IN const char *name, 
                      IN const hlir_t *of, 
                      hlir_type_t type);
