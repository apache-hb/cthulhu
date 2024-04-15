src = [
    'src/meta.c',

    'src/emit/emit.c',
    'src/emit/cmdline.c'
]

deps = [
    memory, events, io, config, setup,
    argparse, broker, arena, format
]

inc = include_directories('include')

libmeta = library('meta', src,
    c_args : generated_args,
    build_by_default : not meson.is_subproject(),
    include_directories : inc,
    dependencies : deps
)

meta = declare_dependency(
    link_with : libmeta,
    include_directories : inc
)
