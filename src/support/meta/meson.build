src = [
    'src/main.c',

    'src/emit/emit.c',
    'src/emit/cmdline.c'
]

deps = [
    memory, events, io, config, setup,
    argparse, broker, arena
]

meta = executable('meta', src,
    c_args : generated_args,
    build_by_default : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : deps
)

ast = generator(meta,
    output : [ '@BASENAME@_ast.h', '@BASENAME@_ast.c' ],
    arguments : [ '/header:@OUTPUT0@', '/source:@OUTPUT1@', '@INPUT@', '/mode:ast' ],
)
