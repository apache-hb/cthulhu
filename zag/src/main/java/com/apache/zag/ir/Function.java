package com.apache.zag.ir;

import java.util.List;
import java.util.stream.Collectors;

public class Function implements Node {
    String name;
    List<Argument> args;
    Type result;
    Stmt body;
    
    public Function(String id, Type res, List<Argument> params, Stmt stmt) {
        name = id;
        args = params;
        result = res;
        body = stmt;
    }

    public static class Argument implements Node {
        String name;
        Type type;

        public Argument(String id, Type of) {
            name = id;
            type = of;
        }

        @Override
        public String emit() {
            return type.emit() + " " + name;
        }
    }

    @Override
    public String emit() {
        return result.emit() + " " + name + "(" + args.stream().map(Argument::emit).collect(Collectors.joining(", ")) + ") {" + body.emit() + "}";
    }
}
