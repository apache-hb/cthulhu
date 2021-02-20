package com.apache.zag.ir;

import java.math.BigInteger;

public abstract class Expr extends Stmt {
    public static enum Op {
        ADD,
        SUB,
        MUL,
        DIV,
        MOD;

        String emit() {
            switch (this) {
                case ADD: return "+";
                case SUB: return "-";
                case MUL: return "*";
                case DIV: return "/";
                case MOD: return "%";
                default: return "@";
            }
        }
    }

    public static class Name extends Expr {
        String name;

        public Name(String id) {
            name = id;
        }

        @Override
        public String emit() {
            return name;
        }
    }

    public static abstract class Const extends Expr {
        
    }

    public static class Int extends Const {
        BigInteger num;

        public Int(String n) {
            num = new BigInteger(n);
        }

        @Override
        public String emit() {
            return num.toString();
        }
    }

    public static class Bool extends Const {
        boolean val;

        @Override
        public String emit() {
            return val ? "true" : "false";
        }
    }

    public static class Str extends Const {
        String str;

        @Override
        public String emit() {
            return "\"" + str + "\"";
        }
    }

    public static class Unary extends Expr {
        Op op;
        Expr expr;

        @Override
        public String emit() {
            return op.emit() + " " + expr.emit();
        }
    }

    public static class Binary extends Expr {
        Op op;
        Expr lhs;
        Expr rhs;

        public Binary(Op o, Expr l, Expr r) {
            op = o;
            lhs = l;
            rhs = r;
        }

        @Override
        public String emit() {
            return lhs.emit() + " " +  op.emit() + " " + rhs.emit();
        }
    }

    public static class Ternary extends Expr {
        Expr cond;
        Expr truthy;
        Expr falsey;

        @Override
        public String emit() {
            return cond.emit() + " ? " + truthy.emit() + " : " + falsey.emit();
        }
    }
}
