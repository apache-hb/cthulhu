src = [ 'main.c' ]

harness = executable('harness', src,
    dependencies : [ mediator, argparse, support, ssa, emit, check, fs, location ],
    export_dynamic : true
)
