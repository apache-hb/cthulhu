package com.apache.zag.parse;

import com.apache.zag.Tools;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

public class TypeTest {
    private static void compileType(String code) {
        Tools.compileString(code, parser -> parser.type());
    }

    @Test
    @DisplayName("unbounded array")
    void parseUnboundedArray() {
        compileType("[int]");
        compileType("[[int]]");
        compileType("[int]");
        compileType("[[int]]");
        compileType("*[int]");
    }

    @Test
    @DisplayName("bounded array")
    void parseBoundedArray() {
        compileType("[int:16]");
        compileType("[[int:16]]");
        compileType("[[int:8]:82]");
        compileType("*[int:64]");
        compileType("[[int]:128]");
    }

    @Test
    @DisplayName("closure signatures")
    void parseClosures() {
        // TODO: these are ambigous but im not sure why
        compileType("[int()]");
        compileType("[int([int])]");
        compileType("[int():2]");
        compileType("[int([int:32])]");
        compileType("int()");
        compileType("int(int, int)");
        compileType("[int](int)");
    }

    @Test
    @DisplayName("qualified names")
    void parseQualifiedNames() {
        compileType("name::part");
        compileType("*nested::type");
        compileType("type!<arg>");
        compileType("type!<arg>::name");
        compileType("type::name!<arg>");
        compileType("type::name!<arg1, arg2, arg3>");
        compileType("type!<lhs, rhs>::nested!<one, two, three>");
    }

    @Test
    @DisplayName("pointers")
    void parsePointers() {
        compileType("*pointer");
        compileType("******deep");
        compileType("*[array:64]");
        compileType("*type::name!<arg>");
        compileType("name!<*arg, ****arg2>");
    }
}
