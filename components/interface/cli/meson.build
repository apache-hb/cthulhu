src = [
    'src/cmd.c',
    'main.c'
]

cli = executable('ctc-cli', src,
    link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
    include_directories : [ 'include' ],
    dependencies : [ generic, mediate, argparse, ssa, emit ]
)
