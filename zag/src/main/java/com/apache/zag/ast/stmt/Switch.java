package com.apache.zag.ast.stmt;

import java.util.List;

import com.apache.zag.ast.Node;
import com.apache.zag.ast.expr.Expr;

public class Switch extends Stmt {
    class Case extends Node {
        Expr expr;
        Stmt body;
    }

    List<Case> cases;
    Stmt other;
}
