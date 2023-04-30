clis = {}
foreach name, lib : langs
    exe = executable(name, 'interface/main.c',
        link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
        dependencies : [ lib, generic, mediate, argparse, argparse2, ssa, emit ]
    )

    clis += { name : exe }
endforeach

cli = executable('ctu-cli', 'interface/main2.c',
    link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
    dependencies : [ generic, mediate, argparse2, ssa, emit ]
)
