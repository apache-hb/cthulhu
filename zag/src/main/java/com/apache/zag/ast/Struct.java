package com.apache.zag.ast;

import java.util.List;

public class Struct extends Decl {
    List<Decorator> attribs;
    List<Decl> inherits;
}
