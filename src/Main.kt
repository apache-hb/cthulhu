import java.io.*;

enum class Keyword {
    TYPE,
    STRUCT,
    IMPORT,

    LPAREN,
    RPAREN,
    LSQUARE,
    RSQUARE,
    LBRACE,
    RBRACE,

    COMMA,
    DOT,
    SEMICOLON,

    AT, // @

    COLON, // :
    COLON2, // ::

    ADD,
    ADDEQ,

    SUB,
    SUBEQ,

    DIV,
    DIVEQ,

    MOD,
    MODEQ,

    MUL,
    MULEQ;
}

open class Token()

class Str(val str: String) : Token()
class Key(val key: Keyword) : Token()
class Ident(val id: String) : Token()
class End() : Token()
class Err(val msg: String) : Token()

fun Char.isIdent(): Boolean = this in 'a'..'z' || this in 'A'..'Z' || this == '_'

class Lexer(var stream: FileInputStream) {
    private var ahead: Int = stream.read()

    private fun collect(first: Char, block: (Char) -> Boolean): String {
        var buf = mutableListOf(first)

        while (block(peek().toChar()))
            buf.add(read().toChar())

        return buf.joinToString("")
    }

    private fun parseIdent(name: String): Token {
        return when (name) {
            "type" -> Key(Keyword.TYPE)
            "struct" -> Key(Keyword.STRUCT)
            "import" -> Key(Keyword.IMPORT)
            else -> Ident(name)
        }
    }

    private fun skip(predicate: (Int) -> Boolean): Int {
        var n = read()
        while (predicate(n))
            n = read()

        return n
    }

    fun next(): Token {
        var n = skip({ it.toChar().isWhitespace() })

        if (n == -1)
            return End()

        return when (val c = n.toChar()) {
            in 'a'..'z', in 'A'..'Z', '_' -> parseIdent(collect(c, { it.isIdent() || it in '0'..'9' }))
            '(' -> Key(Keyword.LPAREN)
            ')' -> Key(Keyword.RPAREN)
            '[' -> Key(Keyword.LSQUARE)
            ']' -> Key(Keyword.RSQUARE)
            '{' -> Key(Keyword.LBRACE)
            '}' -> Key(Keyword.RBRACE)
            ',' -> Key(Keyword.COMMA)
            '.' -> Key(Keyword.DOT)
            ';' -> Key(Keyword.SEMICOLON)
            ':' -> Key(if (consume(':')) Keyword.COLON2 else Keyword.COLON)
            '+' -> Key(if (consume('=')) Keyword.ADDEQ else Keyword.ADD)
            '-' -> Key(if (consume('=')) Keyword.SUBEQ else Keyword.SUB)
            '*' -> Key(if (consume('=')) Keyword.MULEQ else Keyword.MUL)
            '%' -> Key(if (consume('=')) Keyword.MODEQ else Keyword.MOD)
            else -> Err("invalid char ${c}")
        }
    }

    private fun consume(c: Char): Boolean {
        if (peek() == c.toInt()) {
            read()
            return true
        }
        return false
    }

    private fun read(): Int {
        val temp = ahead
        ahead = stream.read()
        return temp
    }

    private fun peek(): Int = ahead
}

fun Token.toString2(): String {
    return if (this is Err)
        "Err(${this.msg})"
    else
        this.toString()
}

open class Node()

open class Expr : Node()

class Intrin(val name: List<String>, val args: List<Expr>) : Node()
class Import(val path: List<String>, val exports: List<String>) : Node()

class Parser(val lex: Lexer) {
    var ahead: Token? = null

    private fun next(): Token {
        var tok = ahead
        ahead = null

        if (tok == null)
            tok = lex.next()

        return tok
    }

    private fun ident(): String? {
        val tok = next()

        assert(tok is Ident)

        return (tok as Ident).id
    }

    private fun consume(key: Keyword): Boolean {
        val tok = next()

        if (tok is Key && tok.key == key)
            return true

        ahead = tok
        return false
    }

    private fun expectKey(key: Keyword) {
        val tok = next()

        assert(tok is Key)
        assert((tok as Key).key == key)
    }

    fun qualId(): List<String> {
        var buf = mutableListOf(ident()!!)
        while (consume(Keyword.COLON2))
            buf.add(ident()!!)

        return buf
    }

    fun intrin(): Intrin? {
        return if (consume(Keyword.AT))
            Intrin(qualId(), listOf())
        else
            null
    }

    fun importArgs(): List<String> {
        fun argBody(): List<String> {
            var buf = mutableListOf<String>()
            buf.add(ident()!!)

            while (consume(Keyword.COMMA))
                buf.add(ident()!!)

            return buf
        }

        expectKey(Keyword.LPAREN)

        var buf = if (consume(Keyword.MUL)) listOf<String>() else argBody()

        expectKey(Keyword.RPAREN)

        expectKey(Keyword.SEMICOLON)

        return buf
    }

    fun parseImport(): Import? {
        return if (consume(Keyword.IMPORT))
            Import(qualId(), importArgs())
        else
            null
    }
}

fun main(args: Array<String>) {
    val lex = Lexer(FileInputStream(args[0]))

    val parse = Parser(lex)

    val decl = parse.parseImport()
    println(decl)
    val decl2 = parse.parseImport()
    println(decl2)
}