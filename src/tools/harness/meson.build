src = [ 'main.c' ]

harness = executable('harness', src,
    dependencies : [
        broker, argparse, support,
        ssa, check, io, fs, setup,
        scan, notify, memory, events, format,
        arena
    ]
)
