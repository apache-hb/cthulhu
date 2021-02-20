package com.apache.zag.ir;

import java.util.ArrayList;
import java.util.List;

import com.apache.zag.ir.Type.Aggregate;

// include* EOF
public class Root implements Node {
    public List<Include> includes = new ArrayList<>();
    public List<Alias> aliases = new ArrayList<>();
    public List<Aggregate> aggregates = new ArrayList<>();
    public List<Function> functions = new ArrayList<>();

    @Override
    public String emit() {
        StringBuilder builder = new StringBuilder();

        builder.append("#include <stddef.h>\n");
        builder.append("#include <stdint.h>\n");

        for (Include include : includes) {
            builder.append(include.emit() + "\n");
        }

        for (Alias alias : aliases) {
            builder.append(alias.emit() + "\n");
        }

        for (Aggregate aggregate : aggregates) {
            builder.append(aggregate.emit() + "\n");
        }

        for (Function function : functions) {
            builder.append(function.emit() + "\n");
        }

        return builder.toString();
    }
}
