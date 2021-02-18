package com.apache.zag.ast.type;

import java.util.List;

public class Closure extends Type {
    Type result;
    List<Type> args;
}
