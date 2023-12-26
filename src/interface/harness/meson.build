src = [ 'main.c' ]

harness = executable('harness', src,
    dependencies : [
        runtime, argparse, support,
        ssa, emit, check, io, fs,
        scan, notify, memory, events
    ],
    export_dynamic : true
)
