src = [ 'gen/main.c', 'gen/gen.c', 'gen/emit.c', 'gen/util.c', lex.process('gen/gen.l'), parse.process('gen/gen.y') ]

cjson = subproject('cjson').get_variable('json')

gentool = executable('gentool', src,
    include_directories : 'gen',
    dependencies : [ gmp, base, std, generic, argparse, cjson ]
)

gencc = generator(gentool,
    output : [ '@BASENAME@-out.y', '@BASENAME@-out.l', '@BASENAME@-api.h', '@BASENAME@-api.c' ],
    arguments : [
        '@INPUT@',
        '@OUTPUT@',
        '--compiler'
    ]
)
