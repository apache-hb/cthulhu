from rply import LexerGenerator

def lexer() -> LexerGenerator:
    lg = LexerGenerator()


    # mathmatical ops
    lg.add('ADD', r'\+')
    lg.add('ADDEQ', r'\+=')

    lg.add('SUB', r'\-')
    lg.add('SUBEQ', r'\-=')

    lg.add('MUL', r'\*')
    lg.add('MULEQ', r'\*=')

    lg.add('DIV', r'\/')
    lg.add('DIVEQ', r'\/=')

    lg.add('MOD', r'\%')
    lg.add('MODEQ', r'\%=')


    # bitwise ops
    lg.add('SHL', r'<<')
    lg.add('SHLEQ', r'<<=')
    
    lg.add('SHR', r'>>')
    lg.add('SHREQ', r'>>=')

    lg.add('BITAND', r'\&')
    lg.add('BITANDEQ', r'\&=')

    lg.add('BITOR', r'\|')
    lg.add('BITOREQ', r'\|=')

    lg.add('BITNOT', r'\~')

    # comparison ops
    lg.add('EQ', r'==')
    lg.add('NEQ', r'\!=')
    lg.add('NOT', r'\!')

    lg.add('AND', r'\&\&')
    lg.add('OR', r'\|\|')

    lg.add('GT', r'>')
    lg.add('GTE', r'>=')

    lg.add('LT', r'<')
    lg.add('LTE', r'<=')

    # language ops
    lg.add('ASSIGN', r':=')

    lg.add('ARROW', r'=>')

    lg.add('COLON2', r'::')
    lg.add('COLON', r':')
    lg.add('COMMA', r',')
    lg.add('DOT', r'\.')

    lg.add('LPAREN', r'\(')
    lg.add('RPAREN', r'\)')
    
    lg.add('LSQUARE', r'\[')
    lg.add('RSQUARE', r'\]')

    lg.add('LBRACE', r'\{')
    lg.add('RBRACE', r'\}')

    # keywords
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


    lg.add('TRUE', r'true')
    lg.add('FALSE', r'false')
    lg.add('NULL', r'null')

    # numbers
    lg.add('HEX', r'0x[0-9a-fA-F]+')
    lg.add('BIN', r'0b[0-1]+')
    lg.add('NUM', r'[\d.]+')

    lg.add('IDENT', r'[_a-zA-Z]([_a-zA-Z0-9]*)')

    # ignore whitespace
    lg.ignore(r'\s+')

    return lg