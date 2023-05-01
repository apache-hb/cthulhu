query = executable('ctc-query', 'src/main.c',
    link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
    dependencies : [ generic, mediate, argparse2, ssa, emit ]
)
