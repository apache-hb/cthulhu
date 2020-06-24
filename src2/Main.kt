import java.io.*;

fun main(args: Array<String>) {
    val parse = Parser(Lexer(FileInputStream(args[1])))
}