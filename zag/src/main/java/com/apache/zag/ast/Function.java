package com.apache.zag.ast;

import java.util.List;

import com.apache.zag.ast.expr.Expr;
import com.apache.zag.ast.stmt.Stmt;
import com.apache.zag.ast.type.Type;

public class Function extends Node {
    class Argument extends Node {
        Ident name;
        Type type;
        Expr value;
    }

    List<Decorator> attribs;
    List<Argument> args;
    Type result;
    Stmt body;
}
