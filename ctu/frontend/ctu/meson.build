src = [ 
    lex.process('ctu.l'),
    parse.process('ctu.y'),
    'scan.c', 'driver.c', 'ast.c', 
    'sema/sema.c', 'sema/value.c', 'sema/data.c',
    'sema/expr.c', 'sema/type.c', 'sema/define.c'
]

ctc = executable('ctc', src,
    include_directories : '.',
    dependencies : options,
    install : true
)
