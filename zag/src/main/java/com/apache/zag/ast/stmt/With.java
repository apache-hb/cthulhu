package com.apache.zag.ast.stmt;

import com.apache.zag.ast.expr.Expr;

import org.antlr.runtime.Token;

public class With extends Stmt {
    Expr init;
    Stmt body;
}
