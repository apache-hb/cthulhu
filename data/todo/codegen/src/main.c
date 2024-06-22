#include "codegen/codegen.h"

#include "core/macros.h"
#include "io/console.h"
#include "io/io.h"

#include "memory/memory.h"
#include "os/os.h"
#include "stacktrace/stacktrace.h"

int main(int argc, const char **argv)
{
    CTU_UNUSED(argc);
    CTU_UNUSED(argv);

    arena_t *arena = ctu_default_alloc();
    init_global_arena(arena);
    init_gmp_arena(arena);

    bt_init();
    os_init();

    io_t *io = io_stdout(arena);

    cg_context_t *ctx = cg_context_new("tree", arena);
    cg_class_t *root = cg_root_class(ctx);
    cg_class_add_basic_field(root, "node", "const node_t*");

    cg_class_t *decl = cg_class_new("decl", root);
    cg_class_add_basic_field(decl, "name", "const char*");

    cg_class_t *type = cg_class_new("type", decl);


    cg_class_t *stmt = cg_class_new("stmt", root);
    cg_class_t *expr = cg_class_new("expr", stmt);
    cg_class_add_class_field(expr, "type", type);

    /// statements

    cg_class_t *block_stmt = cg_class_new("block_stmt", stmt);
    cg_class_add_basic_field(block_stmt, "stmts", "vector_t*");

    cg_class_t *branch_stmt = cg_class_new("branch_stmt", stmt);
    cg_class_add_class_field(branch_stmt, "cond", expr);
    cg_class_add_class_field(branch_stmt, "then", stmt);
    cg_class_add_class_field(branch_stmt, "else", stmt);

    cg_class_t *loop_stmt = cg_class_new("loop_stmt", stmt);
    cg_class_add_class_field(loop_stmt, "cond", expr);
    cg_class_add_class_field(loop_stmt, "body", stmt);

    cg_class_t *assign_stmt = cg_class_new("assign_stmt", stmt);
    cg_class_add_class_field(assign_stmt, "dst", expr);
    cg_class_add_class_field(assign_stmt, "src", expr);

    cg_class_t *return_stmt = cg_class_new("return_stmt", stmt);
    cg_class_add_class_field(return_stmt, "return", expr);

    /// declarations

    cg_class_t *decl_global = cg_class_new("decl_global", decl);
    cg_class_add_basic_field(decl_global, "name", "const char*");
    cg_class_add_class_field(decl_global, "type", type);
    cg_class_add_class_field(decl_global, "value", expr);

    cg_class_t *decl_field = cg_class_new("decl_field", decl);
    cg_class_add_basic_field(decl_field, "name", "const char*");
    cg_class_add_class_field(decl_field, "type", type);

    /// expressions

    cg_class_t *binary_expr = cg_class_new("binary_expr", expr);
    cg_class_add_class_field(binary_expr, "lhs", expr);
    cg_class_add_class_field(binary_expr, "rhs", expr);
    cg_class_add_basic_field(binary_expr, "binary", "binary_t");

    cg_class_t *compare_expr = cg_class_new("compare_expr", expr);
    cg_class_add_class_field(compare_expr, "lhs", expr);
    cg_class_add_class_field(compare_expr, "rhs", expr);
    cg_class_add_basic_field(compare_expr, "compare", "compare_t");

    cg_class_t *unary_expr = cg_class_new("unary_expr", expr);
    cg_class_add_class_field(unary_expr, "expr", expr);
    cg_class_add_basic_field(unary_expr, "unary", "unary_t");

    cg_class_t *call_expr = cg_class_new("call_expr", expr);
    cg_class_add_class_field(call_expr, "callee", expr);
    cg_class_add_basic_field(call_expr, "args", "vector_t*");

    cg_class_t *index_expr = cg_class_new("index_expr", expr);
    cg_class_add_class_field(index_expr, "expr", expr);
    cg_class_add_class_field(index_expr, "index", expr);

    cg_class_t *member_expr = cg_class_new("expr_member", expr);
    cg_class_add_class_field(member_expr, "expr", expr);
    cg_class_add_class_field(member_expr, "member", decl_field);

    /// literals

    cg_class_t *int_literal = cg_class_new("int_literal", expr);
    cg_class_add_basic_field(int_literal, "int_value", "mpz_t");

    cg_class_t *bool_literal = cg_class_new("bool_literal", expr);
    cg_class_add_basic_field(bool_literal, "bool_value", "bool");

    cg_class_t *string_literal = cg_class_new("string_literal", expr);
    cg_class_add_basic_field(string_literal, "string_value", "text_view_t");

    /// types

    cg_class_t *type_param = cg_class_new("type_param", type);
    cg_class_add_class_field(type_param, "type", type);

    cg_class_new("type_unit", type);
    cg_class_new("type_bool", type);
    cg_class_new("type_int", type);
    cg_class_new("type_string", type);

    cg_class_t *type_array = cg_class_new("type_array", type);
    cg_class_add_class_field(type_array, "type", type);
    cg_class_add_class_field(type_array, "size", expr);

    cg_class_t *type_pointer = cg_class_new("type_pointer", type);
    cg_class_add_class_field(type_pointer, "type", type);

    cg_class_t *type_struct = cg_class_new("type_struct", type);
    cg_class_add_basic_field(type_struct, "fields", "vector_t*");

    cg_class_t *type_union = cg_class_new("type_union", type);
    cg_class_add_basic_field(type_union, "fields", "vector_t*");

    cg_class_t *type_function = cg_class_new("type_function", type);
    cg_class_add_basic_field(type_function, "params", "vector_t*");
    cg_class_add_class_field(type_function, "return", type);
    cg_class_add_basic_field(type_function, "variadic", "bool");

    cg_class_t *type_enum = cg_class_new("type_enum", type);
    cg_class_add_basic_field(type_enum, "fields", "vector_t*");

    cg_emit_header(ctx, io);

    io_close(io);
}
