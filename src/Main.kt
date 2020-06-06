import java.io.*;

enum class Keyword {
    TYPE,
    STRUCT,
    IMPORT,
    MUT,

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
            else -> Ident(name)
        }
    }

    private fun skip(predicate: (Int) -> Boolean): Int {
        var n = read()
        while (predicate(n))
            n = read()

        return n
    }

    private fun parseNumber(num: String): Num {
        var buf = collect(read()) { it in '0'..'9' || it == '.' }
        return Num(0.0)
    }

    private fun parseHex(): IntTok {
        var buf = collect(read()) { it.isXDigit() }
        return IntTok(Integer.parseInt(buf, 16))
    }

    private fun parseBin(): IntTok {
        var buf = collect(read()) { it in '0'..'1' }
        return IntTok(Long(buf, 2))
    }

    private fun parseString(): Str {

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

open class Node()

open class Expr : Node()

class Unary(val op: Keyword, val expr: Expr) : Expr()
class IntLit(val v: Long) : Expr()

class Intrin(val name: List<String>, val args: List<Expr>) : Node()
class Import(val path: List<String>, val exports: List<String>) : Node()

open class Type : Node()
class Mut(val of: Type) : Type()
class TypeName(val name: List<String>) : Type()

enum class BuiltinType {
    U8,
    U16,
    U32,
    U64,
    I8,
    I16,
    I32,
    I64,
    F32,
    F64,
    UINT,
    INT,
    BOOL,
    VOID
}
class Builtin(val type: BuiltinType) : Type()
class Ptr(val to: Type) : Type()
class ArrayType(val of: Type, val size: Expr) : Type()

class Struct(val fields: List<Pair<Type, String>>, val attribs: List<Intrin>) : Type()

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

    fun qualId(): List<String> {
        var buf = mutableListOf(ident())
        while (consume(Keyword.COLON2))
            buf.add(ident())

        return buf
    }

    fun intrins(): List<Intrin> {
        if (consume(Keyword.AT)) {
            if (consume(Keyword.LSQUARE)) {
                val items = mutableListOf(Intrin(qualId(), listOf()))
                while (consume(Keyword.COMMA)) {
                    items.add(Intrin(qualId(), listOf()))
                }
                expectKey(Keyword.RSQUARE)
                return items
            }
            return listOf(Intrin(qualId(), listOf()))
        } else {
            return listOf<Intrin>()
        }
    }

    fun importArgs(): List<String> {
        fun argBody(): List<String> {
            var buf = mutableListOf<String>()
            buf.add(ident())

            while (consume(Keyword.COMMA))
                buf.add(ident())

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

    fun parseExpr(): Expr {
        val pre = peek({ it is Key })
        if (pre != null) {
            return Unary((pre as Key).key, parseExpr())
        }

        // TODO: all this
        return Expr()
    }

    fun parseType(): Type {
        fun possiblyBuiltin(name: List<String>): Type {
            return when (name[0]) {
                "u8" -> Builtin(BuiltinType.U8)
                "u16" -> Builtin(BuiltinType.U16)
                "u32" -> Builtin(BuiltinType.U32)
                "u64" -> Builtin(BuiltinType.U64)
                "i8" -> Builtin(BuiltinType.I8)
                "i16" -> Builtin(BuiltinType.I16)
                "i32" -> Builtin(BuiltinType.I32)
                "i64" -> Builtin(BuiltinType.I64)
                "f32" -> Builtin(BuiltinType.F32)
                "f64" -> Builtin(BuiltinType.F64)
                "int" -> Builtin(BuiltinType.INT)
                "uint" -> Builtin(BuiltinType.UINT)
                "bool" -> Builtin(BuiltinType.BOOL)
                "void" -> Builtin(BuiltinType.VOID)
                else -> TypeName(name)
            }
        }

        var out: Type

        if (consume(Keyword.MUT)) {
            expectKey(Keyword.LPAREN)
            out = Mut(parseType())
            expectKey(Keyword.RPAREN)
        } else {
            val id = qualId()
            out = if (id.size == 1) possiblyBuiltin(id) else TypeName(id)
        }

        while (true) {
            if (consume(Keyword.MUL)) {
                out = Ptr(out)
            } else if (consume(Keyword.LSQUARE)) {
                val size = parseExpr()
                expectKey(Keyword.RSQUARE)
                out = ArrayType(out, size)
            } else {
                break
            }
        }

        return out
    }

    fun parseAlias(): Pair<String, Type>? {
        if (consume(Keyword.TYPE)) {
            val name = ident()
            expectKey(Keyword.ASSIGN)
            val type = parseType()
            expectKey(Keyword.SEMICOLON)
            return Pair(name, type)
        } else {
            return null
        }
    }

    fun parseStruct(): Pair<String, Struct>? {
        val attribs = intrins()
        if (consume(Keyword.STRUCT)) {
            val name = ident()
            expectKey(Keyword.LPAREN)

            var fields = mutableListOf<Pair<Type, String>>()

            while (!consume(Keyword.RPAREN)) {
                fields.add(Pair(parseType(), ident()))
                expectKey(Keyword.SEMICOLON)
            }

            return Pair(name, Struct(fields.toList(), attribs))
        }

        return null
    }
}

fun main(args: Array<String>) {
    val lex = Lexer(FileInputStream(args[0]))

    val parse = Parser(lex)

    val decl = parse.parseImport()
    println(decl)
    val decl2 = parse.parseImport()
    println(decl2)

    val decl3 = parse.parseAlias()
    println(decl3)

    val decl4 = parse.parseAlias()
    println(decl4)

    val decl5 = parse.parseStruct()
    println(decl5)
}