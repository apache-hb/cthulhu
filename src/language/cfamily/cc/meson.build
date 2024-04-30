src = [
    'src/ast.c',
    'src/lexer.c',
    'src/parser.c',
    'src/main.c'
]

deps = [ std ]

libcc = library('ctu-cc', src,
    include_directories : 'include',
    dependencies : deps
)
