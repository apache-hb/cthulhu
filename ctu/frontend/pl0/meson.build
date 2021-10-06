src = [ 
    lex.process('pl0.l'), 
    parse.process('pl0.y'),
    'scan.c', 'driver.c', 'ast.c', 'sema.c'
]

pl0 = executable('pl0c', src,
    include_directories : '.',
    dependencies : options,
    install : true
)
