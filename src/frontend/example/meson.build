src = [ 'main.c' ]

example = executable('example', src,
    link_args : setargv,
    export_dynamic : module_loader.contains('shared'),
    dependencies : [
        broker, argparse, support,
        tree, ssa, emit, io, fs,
        scan, notify, memory, events, format, setup, arena, check
    ]
)
