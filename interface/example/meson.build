src = [ 'main.c' ]

example = executable('example', src,
    link_args : setargv,
    dependencies : [ mediator, argparse, support, tree, ssa, emit, io, fs, scan, report, memory ],
    export_dynamic : true
)
