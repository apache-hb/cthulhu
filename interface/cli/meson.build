foreach name, lib : langs
    executable(name, 'interface/main.c',
        link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
        dependencies : [ lib, generic, argparse, ssa, emit ]
    )
endforeach
