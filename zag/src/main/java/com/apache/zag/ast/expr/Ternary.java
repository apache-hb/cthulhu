package com.apache.zag.ast.expr;

public class Ternary extends Expr {
    Expr cond;
    Expr yes;
    Expr no;
}
