package com.apache.zag.ast;

import java.util.List;

import com.apache.zag.ast.expr.Expr;

public class Enum extends Decl {
    class Field extends Node {
        Ident name;
        Expr value;
    }

    List<Field> fields;
    List<Function> functions;
}
