import java.io.*;

data class Position(
    val dist: Long,
    val line: Long,
    val col: Long,
    val stream: FileInputStream
)

abstract class Token()

class Ident(val id: String) : Token()
class Err(val msg: String) : Token()
class Eof() : Token()
class Key(val key: Keyword) : Token()

class Lexer(var stream: FileInputStream) {
    private var ahead = stream.read()

    private fun read(): Int {
        val temp = ahead
        ahead = stream.read()
        return temp
    }

    private fun consume(c: Char): Boolean {
        if (ahead == c.toInt()) {
            ahead = stream.read()
            return true
        }
        return false
    }

    private fun skip(filter: (Int) -> Boolean): Int {
        var c = read()
        while (filter(c))
            c = read()

        return c
    }

    private fun collect(first: Char, filter: (Char) -> Boolean): String {
        val bld = StringBuilder()
        bld.append(first)
        while (filter(ahead.toChar())) {
            bld.append(read())
        }

        return bld.toString()
    }

    fun next(): Token {
        // our lexer skips all whitespace without exception
        // being whitespace sensitive is not a fun time
        val n = skip({ it.toChar().isWhitespace() })

        // if we reach the end of the file then skip
        if (n == -1)
            return Eof()

        val c = n.toChar()

        fun key(c: Char, t: Keyword, f: Keyword)
            = Key(if (consume(c)) t else f)

        fun ident(c: Char)
            = when (val it = collect(c, { it.isLetterOrDigit() || it == '_' })) {
                "import" -> Key(Keyword.IMPORT)
                "let" -> Key(Keyword.LET)
                "var" -> Key(Keyword.VAR)
                "def" -> Key(Keyword.DEF)
                "type" -> Key(Keyword.TYPE)
                "struct" -> Key(Keyword.STRUCT)
                "union" -> Key(Keyword.UNION)
                "enum" -> Key(Keyword.ENUM)
                "if" -> Key(Keyword.IF)
                "else" -> Key(Keyword.ELSE)
                "do" -> Key(Keyword.DO)
                "while" -> Key(Keyword.WHILE)
                "for" -> Key(Keyword.FOR)
                "switch" -> Key(Keyword.SWITCH)
                "case" -> Key(Keyword.CASE)
                "break" -> Key(Keyword.BREAK)
                "default" -> Key(Keyword.DEFAULT)
                "match" -> Key(Keyword.MATCH)
                "return" -> Key(Keyword.RETURN)
                "cast" -> Key(Keyword.CAST)
                "continue" -> Key(Keyword.CONTINUE)
                else -> Ident(it)
            }

        return when (c) {
            '+' -> key('=', Keyword.ADDEQ, Keyword.ADD)
            '-' -> key('=', Keyword.SUBEQ, Keyword.SUB)
            '*' -> key('=', Keyword.MULEQ, Keyword.MUL)
            '/' -> key('=', Keyword.DIVEQ, Keyword.DIV)
            '%' -> key('=', Keyword.MODEQ, Keyword.MOD)
            ':' -> key(':', Keyword.COLON2, Keyword.COLON)
            '!' -> key('=', Keyword.NEQ, Keyword.NOT)
            '=' -> key('=', Keyword.EQ, Keyword.ASSIGN)
            in 'a'..'z', in 'A'..'Z', '_' -> ident(c)
            else -> Err("invalid character $c")
        }
    }
}