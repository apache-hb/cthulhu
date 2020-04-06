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

        'ASSIGN', 'ARROW',
        'LPAREN', 'RPAREN', 'LBRACE', 'RBRACE', 'LSQUARE', 'RSQUARE',

        'IMPORT', 'TYPE', 'VARIANT', 'ENUM', 'UNION',
        'DEF', 'VAR', 'LET', 'RETURN', 'FOR', 'WHILE',
        'MATCH', 'FOR', 'WHILE', 'BREAK', 'CONTINUE',
        'COLON2', 'COLON', 'COMMA', 'DOT',

        'TRUE', 'FALSE', 'NULL', 'IDENT'
    ],
    precedence=[
        ('left', ['ADD', 'SUB']),
        ('left', ['MUL', 'DIV', 'MOD'])
    ],
    cache_id='ctulang')

    return pg