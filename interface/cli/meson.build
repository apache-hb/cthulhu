src = [ 'src/cmd.c', 'main.c' ]

cli = executable('cli', src,
    include_directories : [ 'include' ],
    link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
    dependencies : [ mediator, argparse, support, tree, ssa, emit ],
    export_dynamic : true
)
