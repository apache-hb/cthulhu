package com.apache.zag.ast.expr;

import com.apache.zag.ast.type.Type;

public class Cast extends Expr {
    Expr expr;
    Type to;
}
