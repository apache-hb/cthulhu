src = [ 'src/cmd.c', 'main.c' ]

cli = executable('cli', src,
    include_directories : [ 'include' ],
    link_args : setargv,
    dependencies : [
        memory, runtime, argparse, support,
        tree, ssa, emit, io, fs,
        scan, notify, config, events, defaults
    ],
    export_dynamic : true
)
