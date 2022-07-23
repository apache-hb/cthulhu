src = [ 
    'main.c', 
    lex.process('gen.l'), 
    parse.process('gen.y')
]

gentool = library('gentool', src,
    dependencies : [ deps ]
)
