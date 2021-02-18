package com.apache.zag.ast;

import com.apache.zag.ast.type.Type;

public class Alias extends Node {
    Ident name;
    Type to;
}
