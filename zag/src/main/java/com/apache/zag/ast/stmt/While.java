package com.apache.zag.ast.stmt;

import com.apache.zag.ast.expr.Expr;

public class While extends Stmt {
    Expr cond;
    Stmt body;
}
