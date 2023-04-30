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
    c_args : args,
    include_directories : [ 'src' ]
)

langs += { 'pl0': declare_dependency(link_with : pl0c) }

pl0c = shared_library('pl0c-shared', src,
    dependencies : [ generic, mediate, interop ],
    c_args : args,
    include_directories : [ 'src' ]
)
