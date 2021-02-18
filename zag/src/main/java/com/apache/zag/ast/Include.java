package com.apache.zag.ast;

import java.util.List;

public class Include extends Node {
    List<Ident> path;
    List<Ident> items;
}
