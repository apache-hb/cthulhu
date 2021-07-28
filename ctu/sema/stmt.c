static void build_stmt(sema_t *sema, node_t *stmt);

static void build_stmts(sema_t *sema, node_t *stmts) {
    list_t *nodes = stmts->stmts;

    for (size_t i = 0; i < list_len(nodes); i++) {
        build_stmt(sema, list_at(nodes, i));
    }
}

static void build_assign(sema_t *sema, node_t *assign) {
    type_t *dst = query_expr(sema, assign->dst);
    type_t *src = query_expr(sema, assign->src);

    if (!is_lvalue(dst)) {
        reportf(LEVEL_ERROR, assign, "can only assign to lvalues");
    }

    if (!type_can_become_implicit(&assign->src, dst, src)) {
        reportf(LEVEL_ERROR, assign, "cannot assign unrelated types");
    }
}

static void build_return(sema_t *sema, node_t *stmt) {
    type_t *result = VOID_TYPE;
    if (stmt->expr) {
        result = query_expr(sema, stmt->expr);
    }

    if (!type_can_become_implicit(&stmt->expr, sema->result, result)) {
        reportf(LEVEL_ERROR, stmt, "cannot return incompatible types");
    }

    connect_type(stmt, result);
}

static void local_var(sema_t *sema, node_t *stmt) {
    build_var(sema, stmt);
    add_discard(sema->vars, stmt);
    mark_local(stmt);
}

static void build_branch(sema_t *sema, node_t *stmt) {
    if (stmt->cond) {
        type_t *cond = query_expr(sema, stmt->cond);
        if (!type_can_become_implicit(&stmt->cond, BOOL_TYPE, cond)) {
            reportf(LEVEL_ERROR, stmt, "can only branch on boolean convertible expressions");
        }
    }

    build_stmt(sema, stmt->branch);

    if (stmt->next) {
        build_branch(sema, stmt->next);
    }
}

static void build_while(sema_t *sema, node_t *stmt) {
    type_t *cond = query_expr(sema, stmt->cond);
    build_stmt(sema, stmt->next);

    if (!type_can_become_implicit(&stmt->cond, BOOL_TYPE, cond)) {
        reportf(LEVEL_ERROR, stmt, "can only loop on boolean convertible expressions");
    }
}

static void build_stmt(sema_t *sema, node_t *stmt) {
    switch (stmt->kind) {
    case AST_STMTS:
        build_stmts(new_sema(sema), stmt);
        break;

    case AST_ASSIGN:
        build_assign(sema, stmt);
        break;

    case AST_RETURN:
        build_return(sema, stmt);
        break;

    case AST_DECL_VAR:
        local_var(sema, stmt);
        break;

    case AST_BRANCH:
        build_branch(sema, stmt);
        break;

    case AST_WHILE:
        build_while(sema, stmt);
        break;

    case AST_CALL:
        query_expr(sema, stmt);
        break;

    case AST_SYMBOL: case AST_DIGIT: case AST_BOOL:
    case AST_STRING: case AST_UNARY: case AST_BINARY:
    case AST_CAST: case AST_ACCESS: 
        reportf(LEVEL_ERROR, stmt, "unexpected expression");
        break;

    default:
        assert("unimplemented build-stmt %d", stmt->kind);
        break;
    }
}