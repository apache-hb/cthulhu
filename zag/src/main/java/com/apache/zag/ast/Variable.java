package com.apache.zag.ast;

import java.util.List;

import com.apache.zag.ast.expr.Expr;
import com.apache.zag.ast.type.Type;

public class Variable extends Node {
    class Name extends Node {
        Ident name;
        Type type;
    }

    List<Name> names;
    Expr expr;
}
