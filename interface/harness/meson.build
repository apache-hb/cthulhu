src = [ 'main.c' ]

harness = executable('harness', src,
    dependencies : [ mediator, argparse, support, ssa, emit, check, io, fs, scan, notify, memory ],
    export_dynamic : true
)
