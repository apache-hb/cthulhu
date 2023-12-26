src = [ 'main.c' ]

example = executable('example', src,
    link_args : setargv,
    dependencies : [
        runtime, argparse, support,
        tree, ssa, emit, io, fs,
        scan, notify, memory, events
    ],
    export_dynamic : true
)
