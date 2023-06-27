src = [ 'main.c' ]

example = executable('example', src,
    link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
    dependencies : [ mediator, argparse, support, h2 ]
)
