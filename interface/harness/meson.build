src = [ 'main.c' ]

harness = executable('harness', src,
    dependencies : [ mediator, argparse, support, ssa, emit, check, fs, scan, report, memory ],
    export_dynamic : true
)
