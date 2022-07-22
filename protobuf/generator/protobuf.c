#include "protobuf.h"

#include "base/memory.h"

ast_t *pb_new(scan_t scan, where_t where, astof_t of)
{
    ast_t *ast = ctu_malloc(sizeof(ast_t));
    ast->node = node_new(scan, where);
    ast->kind = of;
    return ast;
}

ast_t *pb_module(scan_t scan, where_t where, vector_t *messages);
ast_t *pb_message(scan_t scan, where_t where, const char *name, vector_t *fields);
ast_t *pb_field(scan_t scan, where_t where, const char *name, uint_least32_t index, const char *type, bool repeated);
ast_t *pb_enum(scan_t scan, where_t where, const char *name, vector_t *fields);
ast_t *pb_case(scan_t scan, where_t where, const char *name, uint_least32_t index);
ast_t *pb_oneof(scan_t scan, where_t where, const char *name, vector_t *fields);
