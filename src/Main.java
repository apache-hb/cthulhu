import java.io.*;
import java.util.*;

public class Main {
    public enum Key {
        DEF,
        TYPE,
        INCLUDE,
        VAR;
    }

    public class FilePos {
        long dist;
        long line;
        long col;
    }

    public class Token {
        FilePos pos;
    }

    public class Keyword extends Token { Key key; }
    public class Str extends Token { String str; }
    public class Ident extends Token { String id; }
    public class Num extends Token { double num; }
    public class Int extends Token { long val; }

    public class Lexer {
        InputStream in;
    }

    public class AST { }

    public class Include extends AST {
        List<String> path;
        String alias;
    }

    public class Stmt extends AST { }

    public class Expr extends Stmt { }

    public class While extends Stmt {
        Expr cond;
        Stmt body;
    }

    public class StmtList extends Stmt {
        List<Stmt> stmts;
    }

    public class Branch extends Stmt {
        List<Pair<Expr, Stmt>> branches;
        Stmt last;
    }

    public class For extends Stmt {
        String loop;
        Expr iter;
        Stmt body;
    }

    public enum BinaryOp {
        ADD,
        ADDEQ,
        SUB,
        SUBEQ,
        MUL,
        MULEQ,
        DIV,
        DIVEQ,
        MOD,
        MODEQ,
        BITAND,
        BITANDEQ,
        BITOR,
        BITOREQ,
        XOR,
        XOREQ,
        GT,
        GTE,
        LT,
        LTE,
        EQ,
        NEQ,
        SHL,
        SLHEQ,
        SHR,
        SHREQ,
        OR,
        AND;
    }

    public class Binary extends Expr {
        BinaryOp op;
        Expr lhs;
        Expr rhs;
    }

    public enum UnaryOp {
        POS,
        NEG,
        NOT,
        FLIP,
        REF,
        DEREF;
    }

    public class Unary extends Expr {
        UnaryOp op;
        Expr expr;
    }

    public class Paren extends Expr {
        Expr body;
    }

    public class Cast extends Expr {
        Expr expr;
        Type to;
    }

    public class Ternary extends Expr {
        Expr cond;
        Expr truthy;
        Expr falsey;
    }

    public class Scope extends Expr {
        String name;
        Expr body;
    }

    public class Access extends Expr {
        Expr body;
        Expr elem;
    }

    public class Deref extends Expr {
        Expr body;
        Expr elem;
    }

    public class Subscript extends Expr {
        Expr expr;
        Expr index;
    }

    public class IntValue extends Expr {
        long num;
    }

    public class FloatValue extends Expr {
        double num;
    }

    public class Call extends Expr {
        Expr func;
        List<Expr> args;
    }

    public class Var extends Stmt {
        String name;
        Type type;
        Expr init;
    }

    public class StructInit extends Stmt {
        Map<String, Expr> values;
    }

    public class ArrayInit extends Stmt {
        List<Expr> values;
    }

    public class Type extends AST { }

    public enum BuiltinType {
        I8,
        I16,
        I32,
        I64,
        U8,
        U16,
        U32,
        U64,
        F32,
        F64,
        VOID;
    }

    public class Builtin extends Type {
        BuiltinType type;
    }

    public class Any extends Type {
        Map<String, Type> fields;
    }

    public class Union extends Type {
        Map<String, Type> fields;
    }

    public class Enum extends Type {
        Type backing;
        Map<String, Expr> values;
    }

    public class Struct extends Type {
        List<Pair<String, Type>> fields;
    }

    public class Pointer extends Type {
        Type to;
    }

    public class Reference extends Type {
        Type to;
    }

    public class Array extends Type {
        Type of;
        Expr size;
    }

    public class Name extends Type {
        List<String> name;
    }

    public class Const extends Type {
        Type of;
    }

    public class Closure extends Type {
        List<Type> args;
        Type ret;
    }

    public class Pair<K, V> {
        K key;
        V val;
    }

    public class Func extends AST {
        Type ret;
        List<Pair<String, Type>> args;
    }

    public class Program extends AST {
        List<Include> deps;
        Map<String, Type> types;
        Map<String, Func> funcs;
    }

    public static void main(String[] args) {

    }
}