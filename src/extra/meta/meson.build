src = [
    lex.process('src/meta.l'),
    parse.process('src/meta.y'),

    'src/main.c',
    'src/scan.c',
    'src/meta.c',
    'src/ast.c'
]

deps = [
    setup, config, memory, io,
    notify, arena, scan, interop,
    events, format
]

metatool = executable('metatool', src,
    c_args : generated_args,
    build_by_default : not meson.is_subproject(),
    include_directories : [ 'include' ],
    dependencies : deps
)

meta = generator(metatool,
    output : [ '@BASENAME@_ast.c', '@BASENAME@_ast.h' ],
    arguments : [ '/source:@OUTPUT0@', '/header:@OUTPUT1@', '/ast:@INPUT@' ]
)
