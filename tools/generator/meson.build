src = [ 'gen/main.c', 'gen/ast.c', lex.process('gen/gen.l'), parse.process('gen/gen.y') ]

generator = executable('generator', src,
    dependencies : [ gmp, ctbase, ctstd ]
)
