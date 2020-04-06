from libctu.lex import lexer
from libctu.parse import parser
import libctu.ast as ast

lg = lexer()
pg = parser()

@pg.production('program : toplevel')
@pg.production('program : toplevel program')
def program(p):
    return [p[0]] if len(p) == 1 else [p[0]] + p[1]

@pg.production('toplevel : typedef-decl')
@pg.production('toplevel : func-decl')
def toplevel(p):
    return p[0]


@pg.production('func-decl : DEF IDENT func-args func-return func-outer-body')
@pg.production('func-decl : DEF IDENT func-args func-outer-body')
@pg.production('func-decl : DEF IDENT func-return func-outer-body')
@pg.production('func-decl : DEF IDENT func-outer-body')
def func_decl(p):
    pass

@pg.production('func-outer-body : ASSIGN')
@pg.production('func-outer-body : LBRACE RBRACE')
def func_body(p):
    pass

@pg.production('func-return : ARROW type-decl')
def func_return(p):
    pass

@pg.production('func-args : LPAREN func-args-body RPAREN')
@pg.production('func-args : LPAREN RPAREN')
def func_args(p):
    pass

@pg.production('func-args-body : IDENT COLON type-decl')
@pg.production('func-args-body : IDENT COLON type-decl COMMA func-args-body')
def func_args_body(p):
    pass




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

@pg.error
def error(tok):
    raise ValueError(f'unexpected token {tok}')

lex = lg.build()
parse = pg.build()

tree = parse.parse(lex.lex('''
type s := { a: int, b: float, c: *int }
type t := (int, float, *double)
type u := union { a: float }
type v := variant { 
    c => float
}

def main(argc: int, argv: **char) => int :=
def main() => int :=
def main() => int {}
def main => int :=
def main => int {}
def main :=
def main {}
def foo => void :=
'''
))

for each in tree:
    print(each)

'''
lg = LexerGenerator()

lg.add('ADD', r'\+')
lg.add('SUB', r'\-')
lg.add('DIV', r'\\')
lg.add('MUL', r'\*')
lg.add('MOD', r'\%')

lg.add('ADDEQ', r'\+=')
lg.add('SUBEQ', r'\-=')
lg.add('DIVEQ', r'\\=')
lg.add('MULEQ', r'\*=')
lg.add('MODEQ', r'\%=')

lg.add('SHL', r'<<')
lg.add('SHR', r'>>')
lg.add('SHLEQ', r'<<=')
lg.add('SHREQ', r'>>=')

lg.add('BITNOT', r'\~')

lg.add('BITAND', r'\&')
lg.add('BITANDEQ', r'\&=')

lg.add('BITOR', r'\|')
lg.add('BITOREQ', r'\|=')

lg.add('ASSIGN', r':=')
lg.add('EQ', r'==')
lg.add('NEQ', r'\!=')
lg.add('NOT', r'\!')

lg.add('ARROW', r'\->')
lg.add('BIGARROW', r'=>')

lg.add('AND', r'\&\&')
lg.add('OR', r'\|\|')
lg.add('GT', r'>')
lg.add('GTE', r'>=')
lg.add('LT', r'<')
lg.add('LTE', r'<=')

lg.add('HEX', r'0x[0-9a-fA-F]+')
lg.add('BIN', r'0b[0-1]+')
lg.add('NUM', r'[\d.]+')

lg.add('LPAREN', r'\(')
lg.add('RPAREN', r'\)')

lg.add('LSQUARE', r'\[')
lg.add('RSQUARE', r'\]')

lg.add('LBRACE', r'\{')
lg.add('RBRACE', r'\}')

lg.add('IMPORT', r'import')
lg.add('TYPE', r'type')
lg.add('VARIANT', r'variant')
lg.add('ENUM', r'enum')
lg.add('UNION', r'union')

lg.add('VAR', r'var')
lg.add('LET', r'let')

lg.add('DEF', r'def')
lg.add('RETURN', r'return')
lg.add('FOR', r'for')
lg.add('WHILE', r'while')
lg.add('MATCH', r'match')
lg.add('BREAK', r'break')
lg.add('CONTINUE', r'continue')

lg.add('COLON2', r'::')
lg.add('COLON', r':')
lg.add('COMMA', r',')
lg.add('DOT', r'\.')

lg.add('TRUE', r'true')
lg.add('FALSE', r'false')
lg.add('NULL', r'null')

lg.add('IDENT', r'[_a-zA-Z]([_a-zA-Z0-9]*)')

lg.ignore(r'\s+')

pg = ParserGenerator([
    'NUM', 'HEX', 'BIN', 
    
    'ADD', 'SUB', 'MUL', 'DIV', 'MOD',
    'ADDEQ', 'SUBEQ', 'MULEQ', 'DIVEQ', 'MODEQ',

    'SHL', 'SHR', 'SHLEQ', 'SHREQ',
    'BITNOT', 'BITAND', 'BITOR', "BITANDEQ", 'BITOREQ',

    'ASSIGN', 'EQ', 'NEQ', 'NOT', 
    'AND', 'OR', 'GT', 'LT', 'GTE', 'LTE',

    'ARROW', 'BIGARROW',
    'LPAREN', 'RPAREN', 'LSQUARE', 'RSQUARE', 'LBRACE', 'RBRACE',

    'IMPORT', 'TYPE', 'VARIANT', 'ENUM', 'UNION', 'DEF', 'VAR', 'LET',
    'RETURN', 'FOR', 'WHILE', 'MATCH', 'BREAK', 'CONTINUE',
    'COLON2', 'COLON', 'COMMA', 'DOT',

    'TRUE', 'FALSE', 'NULL',


    'IDENT'
    ],
    precedence=[
        ('left', ['ADD', 'SUB']),
        ('left', ['MUL', 'DIV', 'MOD'])
    ], 
    cache_id='ctulang'
)

class Typedef:
    def __init__(self, name, t):
        self.name = name
        self.t = t

class Struct:
    def __init__(self, data):
        self.data = data

class Tuple:
    def __init__(self, data):
        self.data = data

class Ptr:
    def __init__(self, data):
        self.data = data

class Builtin:
    def __init__(self, data):
        self.data = data

class Variant:
    def __init__(self, data, backing = Builtin('u32')):
        self.data = data
        self.backing = backing

class Union:
    def __init__(self, data):
        self.data = data

class Enum:
    def __init__(self, data, backing = Builtin('u32')):
        self.data = data
        self.backing = backing

class Variable:
    def __init__(self, name, expr, t = None, const = True):
        self.name = name
        self.t = t
        self.expr = expr
        self.const = const

# toplevel decls

#@pg.production('main : body-decls')
#@pg.production('main : import-decls body-decls')
def main(p):
    pass


# import decls

#@pg.production('import-decls : import-decl')
#@pg.productions('import-decls : import-decl import-decls')
def import_decls(p):
    print(p)
    if len(p) == 1:
        return [p[0]]
    else:
        return [p[0]] + p[1]


#@pg.production('import-decl : IMPORT path')
def import_decl(p):
    return p[1]



# body decls

@pg.production('body-decls : body-decl')
@pg.production('body-decls : body-decl body-decls')
def body_decls(p):
    if len(p) == 1:
        return [p[0]]
    else:
        return [p[0]] + p[1]

@pg.production('body-decl : typedef-decl')
#@pg.production('body-decl : funcdef-decl')
@pg.production('body-decl : let-decl')
def body_decl(p):
    return p[0]

@pg.production('let-decl : LET IDENT ASSIGN expr')
@pg.production('let-decl : LET IDENT COLON type-decl ASSIGN expr')
def let_decl(p):
    if len(p) == 4:
        return Variable(name = p[1].getstr(), expr = p[3])
    else:
        return Variable(name = p[1].getstr(), t = p[3], expr = p[5])

@pg.production('typedef-decl : TYPE IDENT ASSIGN type-decl')
def typedef_decl(p):
    return Typedef(p[1].getstr(), p[3])

@pg.production('type-decl : struct-decl')
@pg.production('type-decl : tuple-decl')
@pg.production('type-decl : ptr-decl')
@pg.production('type-decl : variant-decl')
@pg.production('type-decl : union-decl')
@pg.production('type-decl : enum-decl')
@pg.production('type-decl : path')
def type_decl(p):
    return p[0]


@pg.production('enum-body : IDENT ASSIGN expr')
@pg.production('enum-body : IDENT ASSIGN expr COMMA enum-body')
def enum_body(p):
    if len(p) == 3:
        return [(p[0].getstr(), p[2])]
    else:
        return [(p[0].getstr(), p[2])] + p[4]


@pg.production('enum-decl : ENUM LBRACE RBRACE')
@pg.production('enum-decl : ENUM LBRACE enum-body RBRACE')
def enum_decl(p):
    if len(p) == 3:
        return Enum([])
    else:
        return Enum(p[2])

@pg.production('variant-field : IDENT BIGARROW type-decl')
def variant_field(p):
    return (p[0].getstr(), p[2])

@pg.production('variant-body : variant-field')
@pg.production('variant-body : variant-field COMMA variant-body')
def variant_body(p):
    if len(p) == 1:
        return [p[0]]
    else:
        return [p[0]] + p[2]

@pg.production('variant-decl : VARIANT LBRACE variant-body RBRACE')
@pg.production('variant-decl : VARIANT LBRACE RBRACE')
def variant_decl(p):
    if len(p) == 4:
        return Variant(p[2])
    else:
        return Variant([])


@pg.production('union-decl : UNION LBRACE struct-body RBRACE')
@pg.production('union-decl : UNION LBRACE RBRACE')
def union_decl(p):
    if len(p) == 4:
        return Union(p[2])
    else:
        return Union([])

@pg.production('ptr-decl : MUL type-decl')
def ptr_decl(p):
    return Ptr(p[1])

@pg.production('tuple-body : type-decl')
@pg.production('tuple-body : type-decl COMMA tuple-body')
def tuple_body(p):
    if len(p) == 1:
        return [p[0]]
    else:
        return [p[0]] + p[2]

@pg.production('tuple-decl : LPAREN tuple-body RPAREN')
@pg.production('tuple-decl : LPAREN RPAREN')
def tuple_decl(p):
    if len(p) == 3:
        return Tuple(p[1])
    else:
        return Tuple([])


@pg.production('struct-decl : LBRACE struct-body RBRACE')
@pg.production('struct-decl : LBRACE RBRACE')
def struct_decl(p):
    if len(p) == 3:
        return Struct(p[1])
    else:
        return Struct([])


@pg.production('struct-body : struct-field')
@pg.production('struct-body : struct-field COMMA struct-body')
def struct_body(p):
    if len(p) == 3:
        return [p[0]] + p[2]
    else:
        return [p[0]]


@pg.production('struct-field : IDENT COLON type-decl')
def struct_field(p):
    return (p[0].getstr(), p[2])


@pg.production('path : IDENT')
@pg.production('path : IDENT COLON2 path')
def path(p):
    if len(p) == 1:
        return [p[0]]
    else:
        return [p[0]] + p[2]


@pg.production('expr : expr ADD expr')
@pg.production('expr : expr SUB expr')
@pg.production('expr : expr MUL expr')
@pg.production('expr : expr DIV expr')
@pg.production('expr : expr MOD expr')
@pg.production('expr : expr SHL expr')
@pg.production('expr : expr SHR expr')
@pg.production('expr : expr BITAND expr')
@pg.production('expr : expr BITOR expr')
@pg.production('expr : expr ADDEQ expr')
@pg.production('expr : expr SUBEQ expr')
@pg.production('expr : expr DIVEQ expr')
@pg.production('expr : expr MULEQ expr')
@pg.production('expr : expr MODEQ expr')
@pg.production('expr : expr SHLEQ expr')
@pg.production('expr : expr SHREQ expr')
@pg.production('expr : expr BITANDEQ expr')
@pg.production('expr : expr BITOREQ expr')
@pg.production('expr : expr EQ expr')
@pg.production('expr : expr NEQ expr')
@pg.production('expr : expr AND expr')
@pg.production('expr : expr OR expr')
@pg.production('expr : expr GT expr')
@pg.production('expr : expr GTE expr')
@pg.production('expr : expr LT expr')
@pg.production('expr : expr LTE expr')
def expr1(p):
    return None


@pg.production('expr : LBRACE expr RBRACE')
def expr3(p):
    return None

@pg.production('expr : BITNOT expr')
@pg.production('expr : NOT expr')
@pg.production('expr : ADD expr')
@pg.production('expr : SUB expr')
def expr3(p):
    return None

@pg.production('expr : expr DOT expr')
def expr4(p):
    return None

@pg.production('expr : HEX')
@pg.production('expr : BIN')
@pg.production('expr : NUM')
def expr4(p):
    return None

@pg.error
def error(token):
    raise ValueError(f'unexpected token {token}')

lex = lg.build()
parse = pg.build()

ast = parse.parse(lex.lex('
type x := ()
type y := {}
type thing := u8
type v := variant {}
type u := union {}
type e := enum { name := 0 }
'))

for each in ast:
    print(each)
'''

'''
def main() => int {
    return 5
}

'''