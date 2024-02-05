src = [ 'src/cmd.c', 'main.c' ]

cli = executable('cli', src,
    build_by_default : not meson.is_subproject(),
    export_dynamic : module_loader.contains('shared'),
    include_directories : [ 'include' ],
    link_args : setargv,
    dependencies : [
        memory, broker, argparse, support,
        tree, ssa, emit, io, fs,
        scan, notify, config, events, setup, format,
        arena, check
    ]
)
