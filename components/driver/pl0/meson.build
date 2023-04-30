src = [ 
    'src/main.c', 
    'src/scan.c', 
    'src/ast.c', 
    'src/sema.c',
    lex.process('src/pl0.l'), 
    parse.process('src/pl0.y')
]

pl0 = shared_library('pl0', src,
    dependencies : [ generic, mediate, interop ],
    c_args : args,
    include_directories : [ 'src' ]
)

langs += { 'pl0': pl0 }
