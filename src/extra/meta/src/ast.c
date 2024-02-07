#include "meta/ast.h"

#include "base/panic.h"
#include "std/typed/vector.h"

#include "json/json.h"

ast_decls_t meta_build_decls(const json_t *json, arena_t *arena)
{
    CTASSERT(json != NULL);
    typevec_t *decls = typevec_new(sizeof(ast_node_t), 16, arena);

    ast_decls_t result = {
        .decls = decls,
    };

    return result;
}
