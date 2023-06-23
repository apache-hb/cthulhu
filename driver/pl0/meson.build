src = [ 
    'src/main.c', 
    'src/scan.c', 
    'src/ast.c', 
    'src/sema.c',
    lex.process('src/pl0.l'), 
    parse.process('src/pl0.y')
]

pl0 = library('pl0', src,
    dependencies : [ gmp, generic, mediator, interop, location ],
    c_args : args,
    include_directories : [ 'src' ]
)

langs += { 
    'pl0': {
        'dep': declare_dependency(link_with : pl0),
        'mod': 'kPl0Module'
    }
}
