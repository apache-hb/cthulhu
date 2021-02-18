package com.apache.zag;

import org.antlr.v4.runtime.*;

import com.apache.zag.ZagParser.RootContext;

import static org.junit.jupiter.api.Assertions.assertEquals;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

/**
 * Unit test for simple App.
 */
public class ParseTest {
    /*
    private static void compiles(String str) {
        CharStream stream = CharStreams.fromString(str);
        ZagLexer lexer = new ZagLexer(stream);
        TokenStream tokens = new CommonTokenStream(lexer);
        ZagParser parser = new ZagParser(tokens);

        lexer.addErrorListener(ConsoleErrorListener.INSTANCE);
        parser.addErrorListener(ConsoleErrorListener.INSTANCE);
        RootContext tree = parser.root();

        assertEquals(0, parser.getNumberOfSyntaxErrors());
    }

    @DisplayName("no import list")
    @Test
    public void shouldParseSimpleImport() {
        compiles("using a::b::c;");
    }

    @DisplayName("wildcard import list")
    @Test
    public void shouldParseWildcardImport() {
        compiles("using path::to::items(...);");
    }

    @DisplayName("explicit import list")
    @Test
    public void shouldParseExplicitImport() {
        compiles("using path::to::items(left, right, up, down);");
    }

    @DisplayName("alias a pointer type")
    @Test
    public void shouldParsePointer() {
        compiles("using a = *b;");
    }

    @DisplayName("alias an unbounded array")
    @Test
    public void shouldParseArray() {
        compiles("using a = [int];");
    }

    @Test
    public void shouldParseQual() {
        compiles("using a = a::b;");
    }

    @Test
    public void shouldParseName() {
        compiles("using a = b;");
    }

    @Test
    public void shouldParseFuncSigArgsEmpty() {
        compiles("using a = void();");
    }

    @Test
    public void shouldParseFuncSigArgs() {
        compiles("using a = void(int, int);");
    }

    @Test
    public void shouldParseTemplate() {
        compiles("using a = b!<[x]>;");
    }

    @Test
    public void shouldParseStruct() {
        compiles("struct name {}");
    }

    @Test
    public void shouldParseUnion() {
        compiles("union name {}");
    }

    @Test
    public void shouldParseEnum() {
        compiles("enum name {}");
    }

    @Test
    public void shouldParseFunc() {
        compiles("def name;");
    }

    @Test
    public void shouldParseFuncBody() {
        compiles("def name = a;");
    }

    @Test
    public void shouldParseDecorator() {
        compiles("@opaque struct IUnknown { } ");
    }

    @Test
    public void shouldParseInherit() {
        compiles("""
        struct IUnknown { 

        } 

        struct ID3D12Object : IUnknown {
            
        }
        """);
    }

    @Test
    public void shouldParseMultipleInherit() {
        compiles("""
        struct Writeable {

        }

        struct Readable {

        }

        struct Data : (Load = Readable, Save = Writeable) {

        }
        """);
    }

    @Test
    public void shouldParseDecorators() {
        compiles("""
        @[inline(always), target(i8086)]
        def main: int = 10 + 10;
        """);
    }

    @Test
    public void simpleExpressions() {
        compiles("def main = \"hello world\";");
        compiles("def main = 10 + 10;");
        compiles("def main = cast!<bool>(100);");
    }

    @Test
    public void complexExpressions() {
        compiles("def main = 10 * 5 << (5 / 5) + 5;");
    }

    @Test
    public void fibonacci() {
        compiles(""" 
        def fib1(i: int): int {
            if (i == 0 || i == 1) {
                return i;
            } else {
                return fib1(i - 1) + fib1(i - 2);
            }
        }
        
        def fib2(i: int) = (i == 0 || i == 1) ?: fib2(i - 1) + fib2(i - 2);
        """);
    }

    @Test
    public void unwrapMacro() {
        compiles("""
        def main {
            let x = unwrap! func(10);
        }
        """);
    }*/
}
