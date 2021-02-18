package com.apache.zag.ast.expr;

import java.util.List;

public class Call extends Expr {
    Expr func;
    List<Expr> args;
}
