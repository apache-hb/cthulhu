package com.apache.zag.ast.stmt;

import java.util.List;

import com.apache.zag.ast.expr.Expr;

public class Branch extends Stmt {
    class Path extends Stmt {
        Expr cond;
        Stmt body;
    }

    List<Path> paths;
    Stmt other;
}
