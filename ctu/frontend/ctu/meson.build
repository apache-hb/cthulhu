src = [ 
    lex.process('ctu.l'),
    parse.process('ctu.y'),
    'scan.c', 'driver.c', 'ast.c', 
    'sema/sema.c', 'sema/value.c', 'sema/data.c',
    'sema/expr.c', 'sema/type.c'
]

libctu = library('ctu', src,
    dependencies : generic
)

options += declare_dependency(
    link_with : libctu,
    include_directories : '.'
)
