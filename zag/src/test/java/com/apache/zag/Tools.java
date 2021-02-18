package com.apache.zag;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.io.IOException;
import java.util.function.Function;

import org.antlr.v4.runtime.*;

public class Tools {
    private static ZagParser parseStream(CharStream stream) {
        ZagLexer lexer = new ZagLexer(stream);
        TokenStream tokens = new CommonTokenStream(lexer);
        ZagParser parser = new ZagParser(tokens);

        lexer.addErrorListener(new DiagnosticErrorListener(false));
        parser.addErrorListener(new DiagnosticErrorListener(false));

        return parser;
    }

    public static <T> T compileString(String code, Function<ZagParser, T> consumer) {
        CharStream stream = CharStreams.fromString(code);
        ZagParser parser = parseStream(stream);
        T result = consumer.apply(parser);

        assertEquals(0, parser.getNumberOfSyntaxErrors(), "`" + code + "`");

        return result;
    }

    public static <T> T compileFile(String path, Function<ZagParser, T> consumer) throws IOException {
        CharStream stream = CharStreams.fromFileName(path);
        ZagParser parser = parseStream(stream);
        T result = consumer.apply(parser);

        assertEquals(0, parser.getNumberOfSyntaxErrors(), path);

        return result;
    }
}
