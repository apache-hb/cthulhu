src = [ 'main.c' ]

example = executable('example', src,
    link_args : setargv,
    dependencies : [
        broker, argparse, support,
        tree, ssa, io, fs,
        scan, notify, memory, events, format, setup, arena, check
    ]
)
