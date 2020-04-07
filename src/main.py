from libctu.lex import lexer
from libctu.parse import parser
import libctu.grammar
import libctu.ast as ast


lg = lexer()
pg = parser()

@pg.production('program-decl : import-decls body-decls')
def program_decl1(p):
    return ast.Program(body = p[1], imports = p[0])

@pg.production('program-decl : body-decls')
def program_decl2(p):
    return ast.Program(body = p[0])


#  _                            _
# (_)_ __ ___  _ __   ___  _ __| |_ ___
# | | '_ ` _ \| '_ \ / _ \| '__| __/ __|
# | | | | | | | |_) | (_) | |  | |_\__ \
# |_|_| |_| |_| .__/ \___/|_|   \__|___/
#             |_|


@pg.production('import-decls : import-decl')
def import_decls1(p):
    return [p[0]]

@pg.production('import-decls : import-decl import-decls')
def import_decls2(p):
    return [p[0]] + p[1]

@pg.production('import-decl : IMPORT name-decl')
def import_decl1(p):
    return ast.Import(path = p[1])

@pg.production('import-decl : IMPORT name-decl ARROW IDENT')
def import_decl2(p):
    return ast.Import(path = p[1], alias = p[3].getstr())


#  _               _
# | |__   ___   __| |_   _
# | '_ \ / _ \ / _` | | | |
# | |_) | (_) | (_| | |_| |
# |_.__/ \___/ \__,_|\__, |
#                    |___/

@pg.production('body-decls : body-decl')
def body_decls1(p):
    return [p[0]]

@pg.production('body-decls : body-decl body-decls')
def body_decls2(p):
    return [p[0]] + p[1]


@pg.production('body-decl : typedef-decl')
@pg.production('body-decl : func-decl')
def body_decl(p):
    return p[0]





@pg.production('typedef-decl : TYPE IDENT ASSIGN type-decl')
def typedef_decl(p):
    return ast.Typedef(name = p[1].getstr(), typeof = p[3])


#  _
# | |_ _   _ _ __   ___  ___
# | __| | | | '_ \ / _ \/ __|
# | |_| |_| | |_) |  __/\__ \
#  \__|\__, | .__/ \___||___/
#      |___/|_|

@pg.production('type-decl : struct-decl')
def type_decl(p):
    return p[0]





# a name decl is a List[str]

@pg.production('name-decl : IDENT')
def name_decl1(p):
    return [p[0].getstr()]

@pg.production('name-decl : IDENT COLON2 name-decl')
def name_decl2(p):
    return [p[0].getstr()] + p[2]


@pg.error
def error(tok):
    raise ValueError(f'unexpected token {tok}')

lex = lg.build()
parse = pg.build()

tree = parse.parse(lex.lex('''
import a::b::c => jeff
import ctu::intrinsics => intrin

def main(argc: int, argv: **char) => int {
    if 5 == 6
        return 6
    else if 7 == 8
        return argc
    else 
        return argc + 5
}
'''
))

for each in tree.imports:
    print(each)

for each in tree.body:
    print(each)
