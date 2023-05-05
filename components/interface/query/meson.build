query = executable('ctc-query', 'src/main.c',
    link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
    include_directories : [ versiondir ],
    dependencies : [ generic, mediate, argparse, ssa, emit ]
)
