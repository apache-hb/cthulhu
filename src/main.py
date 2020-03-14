from rply import ParserGenerator, LexerGenerator
from rply.token import BaseBox

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

lg.add('COLON2', r'::')

lg.add('IDENT', r'[_a-zA-Z]([_a-zA-Z0-9]*)')

lg.ignore(r'\s+')

pg = ParserGenerator([
    'NUM', 'HEX', 'BIN', 'ADD', 'SUB', 'MUL', 'DIV', 'MOD', 
    'LPAREN', 'RPAREN', 'IDENT', 'COLON2', 'IMPORT',
    'TYPE', 'VARIANT', 'ENUM', 'UNION'],
    precedence=[
        ('left', ['ADD', 'SUB']),
        ('left', ['MUL', 'DIV', 'MOD'])
    ], 
    cache_id='ctulang'
)

class BoxInt(BaseBox):
    def __init__(self, value):
        self.value = value

    def getint(self):
        return self.value

class Import:
    def __init__(self, path):
        self.path = path

class Func:
    def __init__(self, decl):
        self.decl = decl


class Tuple:
    def __init__(self, fields):
        self.fields = fields

class Struct:
    def __init__(self, fields):
        self.fields = fields

class Variant:
    def __init__(self, fields):
        self.fields = fields

class Enum:
    def __init__(self, fields):
        self.fields = fields

class Main:
    def __init__(self, imports = None, body = None):
        self.imports = imports
        self.body = body

# toplevel

@pg.production('main : import-decls')
def main(p):
    return Main(imports = p[0])

@pg.production('main : body-decls')
def main1(p):
    return Main(body = p[0])

@pg.production('main : import-decls body-decls')
def main2(p):
    return Main(imports = p[0], body = p[1])

# body

@pg.production('body-decls : body-decl')
def body(p):
    return [p[0]]

@pg.production('body-decls : body-decl body-decls')
def body1(p):
    return p[0] + p[1]

@pg.production('body-decl : LPAREN RPAREN')
def body2(p):
    return None

# imports

@pg.production('import-decls : import-decl')
def import_decls(p):
    return [p[0]]

@pg.production('import-decls : import-decl import-decls')
def import_decls2(p):
    return [p[0]] + p[1]

@pg.production('import-decl : IMPORT path')
def import_decl(p):
    return Import([str(part.getstr()) for part in p[1]])


# dotted-name
@pg.production('path : IDENT')
def path(p):
    return [p[0]]

@pg.production('path : IDENT COLON2 path')
def path2(p):
    return [p[0]] + p[2]



@pg.production('expr : expr ADD expr')
@pg.production('expr : expr SUB expr')
@pg.production('expr : expr DIV expr')
@pg.production('expr : expr MOD expr')
@pg.production('expr : expr MUL expr')
def expr_op(p):
    lhs = p[0].getint()
    rhs = p[2].getint()

    tt = p[1].gettokentype()
    if tt == 'ADD':
        return BoxInt(lhs + rhs)
    elif tt == 'SUB':
        return BoxInt(lhs - rhs)
    elif tt == 'MUL':
        return BoxInt(lhs * rhs)
    elif tt == 'DIV':
        return BoxInt(lhs / rhs)
    elif tt == 'MOD':
        return BoxInt(lhs % rhs)

@pg.production('expr : LPAREN expr RPAREN')
def paren_expr(p):
    return p[1]

@pg.production('expr : NUM')
def num_expr(p):
    return p[0].getstr()

@pg.production('expr : BIN')
def bin_expr(p):
    return p[0].getstr()[2:]

@pg.production('expr : HEX')
def hex_expr(p):
    return p[0].getstr()[2:]

@pg.error
def error(token):
    raise ValueError(f'unexpected token {token}')

lex = lg.build()
parse = pg.build()

ast = parse.parse(lex.lex('''
import java::util::HashMap
import x::y::z
'''))