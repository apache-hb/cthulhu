src = [ 
    'src/main.c', 
    'src/scan.c', 
    'src/ast.c', 
    'src/sema.c',
    lex.process('src/pl0.l'), 
    parse.process('src/pl0.y')
]

pl0c = library('pl0c', src,
    dependencies : [ generic, interop ],
    include_directories : [ 'src' ],
    name_suffix: lib_suffix
)

langs += { 'pl0': declare_dependency(link_with : pl0c) }
