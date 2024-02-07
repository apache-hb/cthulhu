src = [
    'src/main.c',
    'src/ast.c'
]

metatool = executable('metatool', src,
    build_by_default : not meson.is_subproject(),
    include_directories : [ 'include' ],
    dependencies : [ json, setup, config, memory, io ]
)

buildast = generator(metatool,
    output : [ '@BASENAME@.c', '@BASENAME@.h' ],
    arguments : [ '/source:@OUTPUT0@', '/header:@OUTPUT1@', '/ast:@INTPUT@' ]
)
