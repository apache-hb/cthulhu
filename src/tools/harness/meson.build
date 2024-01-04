src = [ 'main.c' ]

harness = executable('harness', src,
    dependencies : [
        runtime, argparse, support,
        ssa, emit, check, io, fs,
        scan, notify, memory, events, format
    ],
    export_dynamic : true
)
