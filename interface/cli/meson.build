src = [ 'src/cmd.c', 'main.c' ]

cli = executable('cli', src,
    include_directories : [ 'include' ],
    link_args : setargv,
    dependencies : [ memory, mediator, argparse, support, tree, ssa, emit, io, fs, scan, notify ],
    export_dynamic : true
)
