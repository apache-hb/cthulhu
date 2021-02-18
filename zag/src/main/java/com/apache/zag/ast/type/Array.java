package com.apache.zag.ast.type;

import com.apache.zag.ast.expr.Expr;

public class Array extends Type {
    Type of;
    Expr size;
}
