package com.apache.zag.ast.stmt;

import com.apache.zag.ast.Variable;
import com.apache.zag.ast.expr.Expr;

public class For extends Stmt {
    class Range extends For {
        Variable names;
        Expr range;
    }

    class Loop extends For {
        Stmt init;
        Expr next;
        Expr end;
    }

    Stmt body;
}
