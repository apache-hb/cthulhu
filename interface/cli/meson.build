src = [ 'src/cmd.c', 'main.c' ]

cli = executable('cli', src,
    include_directories : [ 'include' ],
    link_args : setargv,
    dependencies : [ mediator, argparse, support, tree, ssa, emit, fs ],
    export_dynamic : true
)
