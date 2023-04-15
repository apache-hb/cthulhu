clis = {}
foreach name, lib : langs
    exe = executable(name, 'interface/main.c',
        link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
        dependencies : [ lib, generic, argparse, ssa, emit ]
    )

    clis += { name : exe }
endforeach
