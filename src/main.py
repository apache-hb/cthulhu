from rply import ParserGenerator, LexerGenerator
from rply.token import BaseBox

lg = LexerGenerator()
lg.add('IDENT', r'^[_a-zA-Z]\w*')
lg.add('ADD', r'\+')
lg.add('SUB', r'\-')
lg.add('DIV', r'\\')
lg.add('MUL', r'\*')
lg.add('MOD', r'\%')
lg.add('INT', r'\d+')
lg.add('LPAREN', r'\(')
lg.add('RPAREN', r'\)')

lg.add('COLON2', r'\.')

lg.ignore(r'\s+')

pg = ParserGenerator(['INT', 'ADD', 'SUB', 'MUL', 'DIV', 'MOD', 'LPAREN', 'RPAREN', 'IDENT', 'COLON2'],
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

@pg.production('main : path')
def main(p):
    return p[0]

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

@pg.production('expr : INT')
def expr_int(p):
    return BoxInt(int(p[0].getstr()))

@pg.error
def error(token):
    raise ValueError(f'unexpected token {token}')

lex = lg.build()
parse = pg.build()

print(parse.parse(lex.lex('ab . a')))