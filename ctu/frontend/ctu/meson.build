src = [ 
    lex.process('ctu.l'),
    parse.process('ctu.y'),
    'scan.c', 'driver.c', 'ast.c'
]

libctu = library('ctu', src,
    dependencies : generic
)

options += declare_dependency(
    link_with : libctu,
    include_directories : '.'
)
