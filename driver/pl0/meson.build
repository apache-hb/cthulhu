src = [ 
    'main.c', 
    'scan.c', 
    'ast.c', 
    'sema.c',
    lex.process('pl0.l'), 
    parse.process('pl0.y')
]

pl0c = library('pl0c', src,
    dependencies : [ generic, interop ],
    install : true
)

langs += { 'pl0': declare_dependency(link_with : pl0c) }
