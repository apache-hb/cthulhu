src = [ 'src/cmd.c', 'main.c' ]

executable('cli', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : [ 'include' ],
    link_args : setargv,
    dependencies : [
        memory, broker, argparse, support,
        tree, ssa, io, fs,
        scan, notify, config, events, setup, format,
        arena, check, backtrace
    ]
)
