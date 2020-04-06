from libctu.lex import lexer
from libctu.parse import parser
import libctu.ast as ast

lg = lexer()
pg = parser()

@pg.production('program : imports toplevel')
@pg.production('program : toplevel')
def program(p):
    return ast.Program(imports = p[0], body = p[1]) if len(p) == 2 else ast.Program(body = p[0])


@pg.production('imports : import')
@pg.production('imports : import imports')
def imports(p):
    return [p[0]] if len(p) == 1 else [p[0]] + p[1]


@pg.production('import : IMPORT dotted-name')
@pg.production('import : IMPORT dotted-name ARROW IDENT')
def import_decl(p):
    ret = ast.Import(p[1])

    if len(p) == 4:
        ret.alias = p[3].getstr()

    return ret



@pg.production('dotted-name : IDENT')
@pg.production('dotted-name : IDENT COLON2 dotted-name')
def dotted_name(p):
    return [p[0].getstr()] if len(p) == 1 else [p[0].getstr()] + p[2]


@pg.production('toplevel : body')
@pg.production('toplevel : body toplevel')
def toplevel(p):
    return [p[0]] if len(p) == 1 else [p[0]] + p[1]

@pg.production('body : typedef-decl')
@pg.production('body : func-decl')
def body(p):
    return p[0]


@pg.production('typedef-decl : TYPE IDENT ASSIGN type-decl')
def typedef_decl(p):
    return ast.Typedef(name = p[1].getstr(), t = p[3])


@pg.production('type-decl : struct-decl')
@pg.production('type-decl : tuple-decl')
@pg.production('type-decl : union-decl')
@pg.production('type-decl : variant-decl')
@pg.production('type-decl : ptr-decl')
@pg.production('type-decl : builtin-decl')
def type_decl(p):
    return p[0]


@pg.production('ptr-decl : MUL type-decl')
def ptr_decl(p):
    return ast.Ptr(p[1])

@pg.production('builtin-decl : IDENT')
def builtin_decl(p):
    return ast.Builtin(p[0].getstr())



@pg.production('variant-decl : VARIANT LBRACE variant-body RBRACE')
@pg.production('variant-decl : VARIANT LBRACE RBRACE')
def variant_decl(p):
    return p[2] if len(p) == 4 else ast.Variant()

@pg.production('variant-body : IDENT ARROW type-decl')
@pg.production('variant-body : IDENT ARROW type-decl COMMA variant-body')
def variant_body(p):
    ret = ast.Variant({ p[0].getstr(): p[2] })
    if len(p) == 5:
        ret.update(p[4])

    return ret


@pg.production('union-decl : UNION LBRACE struct-body RBRACE')
@pg.production('union-decl : UNION LBRACE RBRACE')
def union_decl(p):
    return ast.Union(p[2]) if len(p) == 4 else ast.Union()


@pg.production('tuple-decl : LPAREN tuple-body RPAREN')
@pg.production('tuple-decl : LPAREN RPAREN')
def tuple_decl(p):
    return p[1] if len(p) == 3 else ast.Tuple()

@pg.production('tuple-body : type-decl')
@pg.production('tuple-body : type-decl COMMA tuple-body')
def tuple_body(p):
    if len(p) == 1:
        return ast.Tuple([p[0]])
    else:
        return ast.Tuple([p[0]] + p[2])





@pg.production('struct-decl : LBRACE struct-body RBRACE')
@pg.production('struct-decl : LBRACE RBRACE')
def struct_decl(p):
    return ast.Struct(p[1]) if len(p) == 3 else ast.Struct()

@pg.production('struct-body : IDENT COLON type-decl')
@pg.production('struct-body : IDENT COLON type-decl COMMA struct-body')
def struct_body(p):
    field = { p[0].getstr(): p[2] }
    if len(p) == 5:
        field.update(p[4])

    return field



@pg.production('func-decl : DEF IDENT func-args func-return func-outer-body')
@pg.production('func-decl : DEF IDENT func-args func-outer-body')
def func_decl1(p):
    func = ast.Function(name = p[1].getstr(), args = p[2])

    if len(p) == 5:
        func.ret = p[3]
        func.exprs = p[4]
    else:
        func.exprs = p[3]

    return func

@pg.production('func-decl : DEF IDENT func-return func-outer-body')
@pg.production('func-decl : DEF IDENT func-outer-body')
def func_decl2(p):
    func = ast.Function(name = p[1].getstr(), args = None)

    if len(p) == 4:
        func.ret = p[2]
        func.exprs = p[3]
    else:
        func.exprs = p[2]

    return func

@pg.production('func-outer-body : ASSIGN expr')
def func_body1(p):
    return p[1][0]

@pg.production('func-outer-body : LBRACE func-body RBRACE')
def func_body2(p):
    return p[1]

@pg.production('func-outer-body : LBRACE RBRACE')
def func_body2(p):
    return []

@pg.production('func-return : ARROW type-decl')
def func_return(p):
    return p[1]

@pg.production('func-args : LPAREN func-args-body RPAREN')
@pg.production('func-args : LPAREN RPAREN')
def func_args(p):
    return p[1] if len(p) == 3 else {}

@pg.production('func-args-body : IDENT COLON type-decl')
@pg.production('func-args-body : IDENT COLON type-decl COMMA func-args-body')
def func_args_body(p):
    ret = { p[0].getstr(): p[2] }
    
    if len(p) == 5:
        ret.update(p[4])

    return ret


@pg.production('func-body : expr')
def func_body(p):
    return [p[0]]

@pg.production('func-body : expr func-body')
def func_body2(p):
    return [p[0]] + [p[1]]

@pg.production('expr : unary')
@pg.production('expr : binary')
@pg.production('expr : ternary')
@pg.production('expr : cast')
def expr1(p):
    return p[0]

@pg.production('cast : expr AS type-decl')
def cast(p):
    return ast.Cast(p[0], p[2])


@pg.production('unary : ADD expr')
@pg.production('unary : SUB expr')
@pg.production('unary : BITNOT expr')
@pg.production('unary : NOT expr')
@pg.production('unary : MUL expr')
@pg.production('unary : BITAND expr')
def unary(p):
    return ast.Unary(p[0], p[1])



@pg.production('binary : expr ADD expr')
@pg.production('binary : expr ADDEQ expr')
@pg.production('binary : expr SUB expr')
@pg.production('binary : expr SUBEQ expr')
@pg.production('binary : expr MUL expr')
@pg.production('binary : expr MULEQ expr')
@pg.production('binary : expr DIV expr')
@pg.production('binary : expr DIVEQ expr')
@pg.production('binary : expr MOD expr')
@pg.production('binary : expr MODEQ expr')
@pg.production('binary : expr SHL expr')
@pg.production('binary : expr SHLEQ expr')
@pg.production('binary : expr SHR expr')
@pg.production('binary : expr SHREQ expr')
@pg.production('binary : expr BITAND expr')
@pg.production('binary : expr BITANDEQ expr')
@pg.production('binary : expr BITOR expr')
@pg.production('binary : expr BITOREQ expr')
@pg.production('binary : expr EQ expr')
@pg.production('binary : expr NEQ expr')
@pg.production('binary : expr AND expr')
@pg.production('binary : expr OR expr')
@pg.production('binary : expr GT expr')
@pg.production('binary : expr GTE expr')
@pg.production('binary : expr LT expr')
@pg.production('binary : expr LTE expr')
def binary(p):
    return ast.Binary(p[1], p[0], p[2])


@pg.production('ternary : expr QUESTION expr COLON expr')
def ternary(p):
    return ast.Ternary(cond = p[0], true = p[2], false = p[4])

@pg.production('expr : const')
def expr1(p):
    return p[0]

@pg.production('const : TRUE')
def const(p):
    return ast.Const(True)

@pg.production('const : FALSE')
def const2(p):
    return ast.Const(False)

@pg.production('const : NULL')
def const3(p):
    return ast.Const(None)


@pg.production('const : HEX')
def const_hex(p):
    return ast.Const(int(p[0].getstr()[2:], 16))

@pg.production('const : BIN')
def const_bin(p):
    return ast.Const(int(p[0].getstr()[2:], 2))

@pg.production('const : NUM')
def const_hex(p):
    return ast.Const(int(p[0].getstr(), 10))


@pg.production('const : CHAR')
@pg.production('const : STRING')
def const_char(p):
    return ast.Const(p[0].getstr()[:-1])

@pg.error
def error(tok):
    raise ValueError(f'unexpected token {tok}')

lex = lg.build()
parse = pg.build()

tree = parse.parse(lex.lex('''
import a::b::c => jeff
import ctu::intrinsics => intrin

def main(argc: int, argv: **char) => int {
    5
}
'''
))

for each in tree.imports:
    print(each)

for each in tree.body:
    print(each)
