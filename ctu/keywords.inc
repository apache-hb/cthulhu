#ifndef KEY
#   define KEY(...)
#endif

#ifndef OP
#   define OP(...)
#endif

#ifndef RES
#   define RES(...)
#endif

/* all keywords */

/* module keywords */
KEY(kimport, "import")
KEY(kscope, "scope")

/* type keywords */
KEY(ktype, "type")
KEY(kvariant, "variant")
KEY(kenum, "enum")
KEY(kunion, "union")

/* declaration keywords */
KEY(kdef, "def")
KEY(kvar, "var")
KEY(klet, "let")

/* control keywords */
KEY(kmatch, "match")
KEY(kif, "if")
KEY(kelse, "else")
KEY(kfor, "for")
KEY(kwhile, "while")
KEY(kbreak, "break")
KEY(kcontinue, "continue")
KEY(kreturn, "return")

/* casting keyword */
KEY(kas, "as")

/* value keywords */
KEY(ktrue, "true")
KEY(kfalse, "false")
KEY(knull, "null")


OP(kassign, ":=")

/* logical operators */

OP(keq, "==")
OP(kneq, "!=")

OP(klt, "<")
OP(klte, "<=")

OP(kgt, ">")
OP(kgte, ">=")

OP(knot, "!")

OP(kor, "||")
OP(kand, "&&")

/* arithmatic operators */

OP(kadd, "+")
OP(kaddeq, "+=")

OP(ksub, "-")
OP(ksubeq, "-=")

OP(kdiv, "/")
OP(kdiveq, "/=")

OP(kmul, "*")
OP(kmuleq, "*=")

OP(kmod, "%")
OP(kmodeq, "%=")


/* bitwise operators */

OP(kbitnot, "~")

OP(kbitxor, "^")
OP(kbitxoreq, "^=")

OP(kbitand, "&")
OP(kbitandeq, "&=")

OP(kbitor, "|")
OP(kbitoreq, "|=")

OP(kshl, "<<")
OP(kshleq, "<<=")

OP(kshr, ">>")
OP(kshreq, ">>=")

/* syntax operators */

OP(klsquare, "[")
OP(krsquare, "]")

OP(klparen, "(")
OP(krparen, ")")

OP(klbrace, "{")
OP(krbrace, "}")

OP(kcolon, ":")
OP(kcolon2, "::")

OP(kdot, ".")
OP(kcomma, ",")
OP(kquestion, "?")

OP(kfuncsig, "&(")

OP(karrow, "->")
OP(kbigarrow, "=>")

/* reserved keywords */
RES(kstatic, "static")
RES(koperator, "operator")
RES(kprivate, "private")
RES(kprotected, "protected")
RES(kpublic, "public")
RES(kclass, "class")
RES(ktemplate, "template")
RES(kvolatile, "volatile")
RES(ksync, "sync")
RES(ksynchronized, "synchronized")
RES(knamespace, "namespace")
RES(kextern, "extern")
RES(kmodule, "module")
RES(kinclude, "include")
RES(kexport, "export")
RES(kdefault, "default")
RES(kusing, "using")
RES(kuse, "use")
RES(knew, "new")
RES(kdelete, "delete")
RES(katomic, "atomic")
RES(klambda, "lambda")

#ifdef KEY
#   undef KEY
#endif

#ifdef OP
#   undef OP
#endif

#ifdef RES
#   undef RES
#endif
