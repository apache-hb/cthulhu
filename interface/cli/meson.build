src = [ 'src/cmd.c', 'main.c' ]

cli = executable('cli', src,
    link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
    include_directories : [ 'include' ],
    dependencies : [ generic, mediator, argparse, ssa, emit, support ]
)
