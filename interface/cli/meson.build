src = [ 'src/cmd.c', 'main.c' ]

cli = executable('cli', src,
    include_directories : [ 'include' ],
    link_args : setargv,
    dependencies : [ memory, mediator, argparse, support, tree, ssa, emit, fs, location ],
    export_dynamic : true
)
