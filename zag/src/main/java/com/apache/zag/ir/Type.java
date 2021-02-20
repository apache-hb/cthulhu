package com.apache.zag.ir;

import java.util.List;

public abstract class Type implements Node {
    public static class Scalar extends Type {
        String name;

        public Scalar(String id) {
            name = id;
        }

        @Override
        public String emit() {
            return name;
        }
    }

    public static class Pointer extends Type {
        private Type to;

        public Pointer(Type type) {
            to = type;
        }

        @Override
        public String emit() {
            return to.emit() + "*";
        }
    }

    public static abstract class Field implements Node {
        String name;
    }

    public static class Variable extends Field {
        Type type;

        public Variable(String id, Type of) {
            name = id;
            type = of;
        }

        @Override
        public String emit() {
            return type.emit() + " " + name + ";";
        }
    }

    public static abstract class Aggregate extends Type {
        String name;
        List<Field> fields;
    }

    public static class Struct extends Aggregate {
        public Struct(String id, Field... items) {
            name = id;
            fields = List.of(items);
        }

        @Override
        public String emit() {
            String layout = "typedef struct " + name + " {";

            for (Field field : fields) {
                layout += field.emit();
            }

            layout += "} " + name + ";";

            return layout;
        }
    }

    public static class Union extends Aggregate {
        public Union(String id, Field... items) {
            name = id;
            fields = List.of(items);
        }
        
        @Override
        public String emit() {
            String layout = "typedef union " + name + " {";

            for (Field field : fields) {
                layout += field.emit();
            }

            layout += "} " + name + ";";
            return layout;
        }
    }
}
