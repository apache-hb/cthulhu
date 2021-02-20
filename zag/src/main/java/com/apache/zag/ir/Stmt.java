package com.apache.zag.ir;

import java.util.List;
import java.util.stream.Collectors;

public abstract class Stmt implements Node {
    public static class Compound extends Stmt {
        List<Stmt> stmts;

        public Compound(Stmt... all) {
            stmts = List.of(all);
        }

        @Override
        public String emit() {
            return "{" + stmts.stream().map(Stmt::emit).collect(Collectors.joining()) + "}";
        }
    }

    public static class Return extends Stmt {
        Expr expr;

        public Return(Expr it) {
            expr = it;
        }

        @Override
        public String emit() {
            return "return " + expr.emit() + ";";
        }
    }

    public static class While extends Stmt {
        Expr cond;
        Stmt body;

        @Override
        public String emit() {
            return "while (" + cond.emit() + ")" + body.emit();
        }
    }
}
