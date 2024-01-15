src = [ 'main.c' ]

harness = executable('harness', src,
    dependencies : [
        runtime, argparse, support,
        ssa, emit, check, io, fs, setup,
        scan, notify, memory, events, format,
        arena
    ],
    export_dynamic : true
)
