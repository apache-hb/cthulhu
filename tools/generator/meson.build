src = [ 'gen/main.c', 'gen/ast.c', 'gen/emit.c', lex.process('gen/gen.l'), parse.process('gen/gen.y') ]

cjson = subproject('cjson').get_variable('json')

gentool = executable('gentool', src,
    include_directories : 'gen',
    dependencies : [ gmp, ctbase, ctstd, generic, cjson ]
)

gencompiler = generator(gentool,
    output : [ '@BASENAME@-out.y', '@BASENAME@-out.l', '@BASENAME@-api.h', '@BASENAME@-api.c' ],
    arguments : [
        '@INPUT@',
        '@OUTPUT@',
        '--compiler'
    ]
)
