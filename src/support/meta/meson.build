src = [
    'src/main.c',
    'src/meta.c'

    # 'src/emit/emit.c',
    # 'src/emit/cmdline.c'
]

deps = [
    memory, events, io, config, setup,
    argparse, broker, arena, format, json
]

metatool = executable('meta', src,
    c_args : generated_args,
    build_by_default : not meson.is_subproject(),
    dependencies : deps
)

meta = generator(metatool,
    output : [ '@BASENAME@_meta.c', '@BASENAME@_meta.h' ],
    arguments : [ '--source=@OUTPUT0@', '--header=@OUTPUT1@', '@INPUT@' ]
)
