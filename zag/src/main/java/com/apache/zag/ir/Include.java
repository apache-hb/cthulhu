package com.apache.zag.ir;

import java.util.List;
import java.util.stream.Collectors;

public class Include implements Node {
    List<String> path;

    public Include(String... parts) {
        path = List.of(parts);
    }

    @Override
    public String emit() {
        return "#include \"" + path.stream().collect(Collectors.joining("/")) + ".h\"";
    }
}
