#ifndef KEYWORD
#   define KEYWORD(...)
#endif

#ifndef OPERATOR
#   define OPERATOR(...)
#endif

OPERATOR(kinvalid, "")

OPERATOR(klsquare, "[")
OPERATOR(krsquare, "]")
OPERATOR(klsquare2, "[[")
OPERATOR(krsquare2, "]]")

OPERATOR(klparen, "(")
OPERATOR(krparen, ")")

OPERATOR(klbrace, "{")
OPERATOR(krbrace, "}")

OPERATOR(karrow, "->")
OPERATOR(kassign, ":=")

OPERATOR(kcolon, ":")
OPERATOR(kcolon2, ":")

OPERATOR(kcomma, ",")
OPERATOR(kdot, ".")

OPERATOR(kquestion, "?")

OPERATOR(knot, "!")
OPERATOR(kbitnot, "~")


OPERATOR(kbitor, "|")
OPERATOR(kbitoreq, "|=")

OPERATOR(kbitand, "&")
OPERATOR(kbitandeq, "&=")

OPERATOR(kbitxor, "^")
OPERATOR(kbitxoreq, "^=")

OPERATOR(kshl, "<<")
OPERATOR(kshleq, "<<=")

OPERATOR(kshr, ">>")
OPERATOR(kshreq, ">>=")

OPERATOR(keq, "==")
OPERATOR(kneq, "!=")

OPERATOR(kgt, ">")
OPERATOR(kgte, ">=")

OPERATOR(klt, "<")
OPERATOR(klte,  "<=")

OPERATOR(kmul, "*")
OPERATOR(kmuleq, "*=")

OPERATOR(kdiv, "/")
OPERATOR(kdiveq, "/=")

OPERATOR(kmod, "%")
OPERATOR(kmodeq, "%=")

OPERATOR(kadd, "+")
OPERATOR(kaddeq, "+=")

OPERATOR(ksub, "-")
OPERATOR(ksubeq, "-=")


KEYWORD(kusing, "using")
KEYWORD(kstruct, "struct")
KEYWORD(kvariant, "variant")
KEYWORD(kenum, "enum")
KEYWORD(kunion, "union")

KEYWORD(kimport, "import")
KEYWORD(kmodule, "module")

KEYWORD(kif, "if")
KEYWORD(kelse, "else")
KEYWORD(kfor, "for")
KEYWORD(kwhile, "while")
KEYWORD(kswitch, "switch")
KEYWORD(kcase, "case")

KEYWORD(kcontinue, "continue")
KEYWORD(kbreak, "break")
KEYWORD(kreturn, "return")

KEYWORD(kin, "in")
KEYWORD(kas, "as")
KEYWORD(kis, "is")

KEYWORD(klet, "let")
KEYWORD(kvar, "var")
KEYWORD(kdef, "def")

KEYWORD(knull, "null")
KEYWORD(ktrue, "true")
KEYWORD(kfalse, "false")

// reserved keywords

KEYWORD(ksome, "some")
KEYWORD(knone, "none")
KEYWORD(kwhere, "where")
KEYWORD(kwhen, "when")
KEYWORD(kinline, "inline")
KEYWORD(knoinline, "noinline")

KEYWORD(knew, "new")
KEYWORD(kdelete, "delete")
KEYWORD(kcast, "cast")
KEYWORD(ktemplate, "template")
KEYWORD(ktype, "type")
KEYWORD(kself, "self")
KEYWORD(kthis, "this")
KEYWORD(ksuper, "super")

KEYWORD(kclass, "class")
KEYWORD(kconst, "const")
KEYWORD(kfriend, "friend")
KEYWORD(kexport, "export")
KEYWORD(kextern, "extern")
KEYWORD(kexplicit, "explicit")
KEYWORD(katomic, "atomic")
KEYWORD(ksynchronized, "synchronized")
KEYWORD(ksync, "sync")
KEYWORD(kmutable, "mutable")
KEYWORD(koperator, "operator")
KEYWORD(kinit, "init")
KEYWORD(kdeinit, "deinit")
KEYWORD(knamespace, "namespace")
KEYWORD(kscope, "scope")
KEYWORD(kconcept, "concept")
KEYWORD(krequires, "requires")
KEYWORD(kstatic, "static")
KEYWORD(kfinal, "final")
KEYWORD(kasync, "async")
KEYWORD(kawait, "await")
KEYWORD(kyield, "yield")


#ifdef OPERATOR
#   undef OPERATOR
#endif

#ifndef KEYWORD
#   undef KEYWORD
#endif