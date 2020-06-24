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

    private fun skipSpace(): Int {
        var n = skip({ it.toChar().isWhitespace() })

        while (n.toChar() == '/') {
            if (consume('/')) {
                while (n.toChar() != '\n')
                    n = read()
            } else if (consume('*')) {
                var depth = 1

                while (depth != 0) {
                    n = read()
                    if (n.toChar() == '/' && consume('*')) {
                        depth += 1
                    } else if (n.toChar() == '*' && consume('/')) {
                        depth -= 1
                    }
                }
            } else {
                break
            }

            while (n.toChar().isWhitespace())
                n = read()
        }

        return n
    }

    fun next(): Token {
        // our lexer skips all whitespace without exception
        // and also comments
        // being whitespace sensitive is not a fun time
        val n = skipSpace()

        // if we reach the end of the file then return
        if (n == -1)
            return Eof()

        val c = n.toChar()

        fun key(c: Char, t: Keyword, f: Keyword)
            = Key(if (consume(c)) t else f)

        fun ident(c: Char)
            = when (val it = collect(c, { it.isLetterOrDigit() || it == '_' })) {
                "import" -> Key(Keyword.IMPORT)
                "var" -> Key(Keyword.VAR)
                "def" -> Key(Keyword.DEF)
                "alias" -> Key(Keyword.ALIAS)
                "struct" -> Key(Keyword.STRUCT)
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
            ',' -> Key(Keyword.COMMA)
            '.' -> Key(Keyword.DOT)
            ';' -> Key(Keyword.SEMICOLON)
            '(' -> Key(Keyword.LPAREN)
            ')' -> Key(Keyword.RPAREN)
            '{' -> Key(Keyword.LBRACE)
            '}' -> Key(Keyword.RBRACE)
            '[' -> Key(Keyword.LSQUARE)
            ']' -> Key(Keyword.RSQUARE)
            '@' -> Key(Keyword.AT)
            in 'a'..'z', in 'A'..'Z', '_' -> ident(c)
            else -> Err("invalid character $c")
        }
    }
}