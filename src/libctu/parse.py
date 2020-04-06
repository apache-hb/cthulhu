from rply import ParserGenerator

def parser() -> ParserGenerator:
    pg = ParserGenerator([
        'ADD', 'ADDEQ', 'SUB', 'SUBEQ',
        'MUL', 'MULEQ', 'DIV', 'DIVEQ',
        'MOD', 'MODEQ',

        'SHL', 'SHLEQ', 'SHR', 'SHREQ',
        'BITNOT', 'BITAND', 'BITANDEQ',
        'BITOR', 'BITOREQ',

        'AND', 'OR', 'GT', 'GTE', 'LT', 'LTE',
        'EQ', 'NEQ', 'NOT',

        'ASSIGN', 'ARROW', 'PTR',
        'LPAREN', 'RPAREN', 'LBRACE', 'RBRACE', 'LSQUARE', 'RSQUARE',

        'IMPORT', 'TYPE', 'VARIANT', 'ENUM', 'UNION',
        'DEF', 'VAR', 'LET', 'RETURN', 'FOR', 'WHILE',
        'MATCH', 'FOR', 'WHILE', 'BREAK', 'CONTINUE', 'AS',
        'COLON2', 'COLON', 'COMMA', 'DOT', 'QUESTION',

        'HEX', 'BIN', 'NUM', 'CHAR', 'STRING',

        'TRUE', 'FALSE', 'NULL', 'IDENT'
    ],
    precedence=[
        ('left', ['COMMA']),
        ('right', ['QUESTION', 'COLON', 'ADDEQ', 'SUBEQ', 'MULEQ', 'DIVEQ', 'MODEQ', 'SHLEQ', 'SHREQ', 'BITANDEQ', 'BITXOREQ', 'BITOREQ']),
        ('left', ['AND', 'OR']),
        ('left', ['BITAND', 'BITXOR', 'BITOR']),
        ('left', ['EQ', 'NEQ']),
        ('left', ['GT', 'GTE', 'LT', 'LTE']),
        ('left', ['SHL', 'SHR']),
        ('left', ['ADD', 'SUB']),
        ('left', ['MUL', 'DIV', 'MOD']),
        ('right', ['NOT', 'BITNOT', 'AS']),
        ('right', ['DOT', 'PTR']),
        ('left', ['COLON2']),
    ],
    cache_id='ctulang')

    return pg