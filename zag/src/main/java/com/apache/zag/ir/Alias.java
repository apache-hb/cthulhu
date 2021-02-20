package com.apache.zag.ir;

// `using` ID `=` type `;` 
public class Alias implements Node {
    String name;
    Type type;

    public Alias(String id, Type to) {
        name = id;
        type = to;
    }

    @Override
    public String emit() {
        return "typedef " + type.emit() + " " + name + ";\n";
    }
}
