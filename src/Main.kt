import java.io.*;

enum class Keyword {
    TYPE,
    STRUCT,
    IMPORT,
    MUT,
    VAR,
    LET,

    LPAREN,
    RPAREN,
    LSQUARE,
    RSQUARE,
    LBRACE,
    RBRACE,

    COMMA,
    DOT,
    SEMICOLON,

    ASSIGN,
    EQ,
    NEQ,
    NOT,

    BITNOT,
    BITAND,
    BITANDEQ,
    BITOR,
    BITOREQ,

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
class IntTok(val num: Long) : Token()
class Num(val num: Double) : Token()
class End() : Token()
class Err(val msg: String) : Token()

fun Char.isIdent(): Boolean = this in 'a'..'z' || this in 'A'..'Z' || this == '_'
fun Char.isXDigit(): Boolean = this in '0'..'9' || this in 'a'..'z' || this in 'A'..'Z'

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
            "mut" -> Key(Keyword.MUT)
            "var" -> Key(Keyword.VAR)
            "let" -> Key(Keyword.LET)
            else -> Ident(name)
        }
    }

    private fun skip(predicate: (Int) -> Boolean): Int {
        var n = read()
        while (predicate(n))
            n = read()

        return n
    }

    private fun parseNumber(num: String): Token {
        if ('.' in num) {
            return Num(0.0)
        } else {
            return IntTok(Integer.parseInt(num).toLong())
        }
    }

    private fun parseHex(): IntTok {
        var buf = collect(read().toChar()) { it.isXDigit() }
        return IntTok(Integer.parseInt(buf, 16).toLong())
    }

    private fun parseBin(): IntTok {
        var buf = collect(read().toChar()) { it in '0'..'1' }
        return IntTok(Integer.parseInt(buf, 2).toLong())
    }

    private fun parseString(): Str {
        var buf = mutableListOf<Char>()
        var c = read().toChar()
        while (c != '"') {
            buf.add(c)
            c = read().toChar()
        }

        return Str(buf.joinToString(""))
    }

    private fun parseFloat(): Num {
        return Num(0.0)
    }

    fun next(): Token {
        var n = skip({ it.toChar().isWhitespace() })

        if (n == -1)
            return End()

        return when (val c = n.toChar()) {
            in 'a'..'z', in 'A'..'Z', '_' -> parseIdent(collect(c, { it.isIdent() || it in '0'..'9' }))
            in '1'..'9' -> parseNumber(collect(c, { it.isDigit() }))
            '0' -> {
                if (consume('x') || consume('X')) parseHex()
                else if (consume('b') || consume('B')) parseBin()
                else if (consume('.')) parseFloat()
                else Err("integer literals may not begin with 0")
            }
            '"' -> parseString()
            '(' -> Key(Keyword.LPAREN)
            ')' -> Key(Keyword.RPAREN)
            '[' -> Key(Keyword.LSQUARE)
            ']' -> Key(Keyword.RSQUARE)
            '{' -> Key(Keyword.LBRACE)
            '}' -> Key(Keyword.RBRACE)
            ',' -> Key(Keyword.COMMA)
            '.' -> Key(Keyword.DOT)
            ';' -> Key(Keyword.SEMICOLON)
            '@' -> Key(Keyword.AT)
            '=' -> Key(if (consume('=')) Keyword.EQ else Keyword.ASSIGN)
            '!' -> Key(if (consume('=')) Keyword.NEQ else Keyword.NOT)
            ':' -> Key(if (consume(':')) Keyword.COLON2 else Keyword.COLON)
            '+' -> Key(if (consume('=')) Keyword.ADDEQ else Keyword.ADD)
            '-' -> Key(if (consume('=')) Keyword.SUBEQ else Keyword.SUB)
            '*' -> Key(if (consume('=')) Keyword.MULEQ else Keyword.MUL)
            '%' -> Key(if (consume('=')) Keyword.MODEQ else Keyword.MOD)
            else -> Err("invalid char $c")
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

typealias QualId = List<String>

open class Node(val attribs: List<Attrib> = listOf())

open class Type : Node()
class Mut(val of: Type) : Type()
class Ptr(val to: Type) : Type()

open class Expr()
class Const(val tok: Token) : Expr()


class Attrib(val name: QualId, val args: List<Expr>)

class Import(val path: QualId, val items: List<String>)

class Program(val imports: List<Import>, val body: Map<String, Node>)

class Parser(val lex: Lexer) {
    var ahead: Token? = null

    private fun next(): Token {
        var tok = ahead
        ahead = null

        if (tok == null)
            tok = lex.next()

        return tok
    }

    private fun ident(): String {
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

    private fun peek(pred: (Token) -> Boolean): Token? {
        val tok = next()

        if (pred(tok))
            return tok

        ahead = tok
        return null
    }

    fun qualId(): QualId {
        val path = mutableListOf<String>()
        do path.add(ident()) while (consume(Keyword.COLON2))
        return path
    }

    fun parseImportSpec(): List<String> {
        var parts = mutableListOf<String>()
        expectKey(Keyword.LPAREN)

        if (!consume(Keyword.MUL)) {
            do parts.add(ident()) while (consume(Keyword.COMMA))
        }

        expectKey(Keyword.RPAREN)

        return parts
    }

    fun parseImport(): Import {
        val name = qualId()
        val parts = parseImportSpec()
        expectKey(Keyword.SEMICOLON)
        return Import(name, parts)
    }

    fun parseExpr(): Expr {
        val thing = peek({ it is IntTok || it is Num || it is Str })
        if (thing != null) {
            return Const(thing)
        }

        // TODO: uhhhhh
        assert(false)
        return Expr()
    }

    fun parseArgs(): List<Expr> {
        if (consume(Keyword.RPAREN)) {
            return listOf()
        } else {
            var args = mutableListOf<Expr>()
            do args.add(parseExpr()) while (consume(Keyword.COMMA))
            expectKey(Keyword.RPAREN)
            return args
        }
    }

    var attribs: MutableList<Attrib> = mutableListOf()

    private fun takeAttribs(): List<Attrib> {
        val out = attribs
        attribs = mutableListOf()
        return out
    }

    fun parseAttribItem() {
        val name = qualId()
        var args = listOf<Expr>()
        if (consume(Keyword.LPAREN)) {
            args = parseArgs()
        }

        attribs.add(Attrib(name, args))
    }

    fun parseAttrib() {
        if (consume(Keyword.LSQUARE)) {
            do parseAttribItem() while (consume(Keyword.COMMA))
            expectKey(Keyword.RSQUARE)
        } else {
            parseAttribItem()
        }
    }

    fun parseType(): Type {
        var out = Type()
        if (consume(Keyword.MUT)) {
            expectKey(Keyword.LPAREN)
            out = Mut(parseType())
            expectKey(Keyword.RPAREN)
        }

        while (true) {
            if (consume(Keyword.MUL)) {
                out = Ptr(out)
            } else {
                break
            }
        }

        return out
    }

    fun parseAlias(): Pair<String, Type> {
        val name = ident()
        expectKey(Keyword.ASSIGN)
        val type = parseType()
        expectKey(Keyword.SEMICOLON)
        return Pair(name, type)
    }

    fun parseProgram(): Program {
        var imports = mutableListOf<Import>()

        while (consume(Keyword.IMPORT))
            imports.add(parseImport())

        var body = mutableMapOf<String, Node>()
        while (true) {
            if (consume(Keyword.AT)) {
                parseAttrib()
            } else if (consume(Keyword.TYPE)) {
                val (name, type) = parseAlias()
                body.put(name, type)
            } else {
                break
            }
        }

        return Program(imports, body)
    }
}

fun main(args: Array<String>) {
    val lex = Lexer(FileInputStream(args[0]))

    val parse = Parser(lex)

    val tree = parse.parseProgram()
}