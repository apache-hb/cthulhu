
// expressions

open class Node()

open class Stmt()
open class Expr : Stmt()

class StmtList(val stmts: List<Stmt>) : Stmt()

// types

open class Type : Node()

typealias Field = Pair<String, Type>

class Struct(
    val fields: List<Field>
) : Type()

class Tuple(val fields: List<Type>) : Type()

class Func(
    val args: List<Field>,
    val ret: Type?,
    val body: Stmt
) : Node()

// toplevel stuff

class Import(
    val path: List<String>,
    val items: List<String>
)

typealias TypeMap = Map<String, Type>
typealias FuncMap = Map<String, Func>

class Unit(
    val imports: List<Import>,
    val types: TypeMap,
    val funcs: FuncMap
)



class Parser(var lex: Lexer) {
    private var ahead: Token? = null

    private fun next(): Token {
        val tok = ahead ?: lex.next()
        ahead = null
        return tok
    }


    private fun ident(): String {
        return (next() as Ident).id
    }

    private fun consume(key: Keyword): Boolean {
        val tok = next()
        if (tok is Key && tok.key == key) {
            return true
        }
        ahead = tok
        return false
    }

    private fun collect(sep: Keyword): List<String> {
        val items = mutableListOf<String>()

        do items.add(ident()) while (consume(sep))

        return items.toList()
    }

    private fun expect(key: Keyword) {
        val tok = next()
        assert(tok is Key && tok.key == key)
    }

    private fun parseName(): List<String> {
        return collect(Keyword.COLON2)
    }

    private fun parseImportItems(): List<String> {
        return collect(Keyword.COMMA)
    }

    private fun parseImport(): Import {
        val path = parseName()

        expect(Keyword.LPAREN)

        val items = parseImportItems()

        expect(Keyword.RPAREN)

        return Import(path, items)
    }

    private fun parseImports(): List<Import> {
        val imports = mutableListOf<Import>()

        while (consume(Keyword.IMPORT)) {
            imports.add(parseImport())
            expect(Keyword.SEMICOLON)
        }

        return imports
    }

    private fun parseTypeName(): Type {

    }

    private fun parseTuple(): Tuple {
        val fields = mutableListOf<Type>()

        do fields.add(parseType()) while (consume(Keyword.COMMA))

        expect(Keyword.RPAREN)

        return Tuple(fields)
    }

    private fun parseType(): Type {
        val type: Type

        while (true) {
            if (consume(Keyword.MUL)) {
                type =
            }
        }
    }

    private fun parseFuncArgs(): List<Field> {
        if (!consume(Keyword.LPAREN)) {
            return listOf()
        }


    }

    private fun parseFuncReturn(): Type? {
        if (!consume(Keyword.COLON)) {
            return null
        }

        return parseType()
    }

    private fun parseExpr(): Expr {

    }

    private fun parseStmtList(): StmtList {

    }

    private fun parseFuncBody(): Stmt {
        if (consume(Keyword.ASSIGN)) {
            return parseExpr()
        } else {
            return parseStmtList()
        }
    }

    private fun parseFunc(): Pair<String, Func> {
        val name = ident()
        val args = parseFuncArgs()
        val ret = parseFuncReturn()
        val body = parseFuncBody()

        return Pair(name, Func(args, ret, body))
    }

    private fun parseStruct(): Pair<String, Struct> {

    }

    private fun parseAttribute() {

    }

    private fun parseBody(): Pair<TypeMap, FuncMap> {
        val types = mutableMapOf<String, Type>()
        val funcs = mutableMapOf<String, Func>()


        while (true) {
            if (consume(Keyword.DEF)) {
                val (name, func) = parseFunc()
                funcs.put(name, func)
            } else if (consume(Keyword.STRUCT)) {
                val (name, type) = parseStruct()
                types.put(name, type)
            } else if (consume(Keyword.AT)) {
                parseAttribute()
            } else {
                break
            }
        }

        return Pair(types, funcs)
    }

    fun parse(): Unit {
        val imports = parseImports()

        val (types, funcs) = parseBody()

        return Unit(imports, types, funcs)
    }
}