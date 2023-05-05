cli = executable('ctc-cli', 'src/main.c',
    link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
    dependencies : [ generic, mediate, argparse, ssa, emit ]
)
