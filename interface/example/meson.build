src = [ 'main.c' ]

example = executable('example', src,
    link_args : setargv,
    dependencies : [ mediator, argparse, support, tree, ssa, emit, fs, scan, report ],
    export_dynamic : true
)
