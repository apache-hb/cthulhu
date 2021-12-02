src = [ 
    lex.process('bf.l'),
    parse.process('bf.y'),
    'driver.c' 
]

bfc = executable('bfc', src,
    include_directories : '.',
    dependencies : options,
    install : true
)
