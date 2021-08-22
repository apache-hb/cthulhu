src = [ 
    lex.process('pl0.l'), 
    parse.process('pl0.y'),
    'scan.c', 'driver.c', 'ast.c'
]

libpl0 = library('pl0', src,
    include_directories : '.',
    dependencies : generic
)

options += declare_dependency(
    link_with : libpl0,
    include_directories : '.',
    compile_args : '-DPL0_DRIVER=1'
)
