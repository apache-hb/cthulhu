src = [ 'gen/main.c', 'gen/gen.c', 'gen/emit.c', lex.process('gen/gen.l'), parse.process('gen/gen.y') ]

gentool = executable('gentool', src,
    include_directories : 'gen',
    dependencies : [ gmp, base, std, generic, argparse ]
)

gencc = generator(gentool,
    output : [ '@BASENAME@-out.y', '@BASENAME@-out.l', '@BASENAME@-api.h', '@BASENAME@-api.c' ],
    arguments : [
        '@INPUT@',
        '@OUTPUT@',
        '--compiler'
    ]
)
