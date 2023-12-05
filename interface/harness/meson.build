src = [ 'main.c' ]

harness = executable('harness', src,
    dependencies : [ mediator, argparse, support, ssa, emit, check, fs, location, memory ],
    export_dynamic : true
)
