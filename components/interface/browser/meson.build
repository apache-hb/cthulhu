foreach name, lib : langs
    exec = executable(name, 'interface/main.c',
        dependencies : [ lib, generic, argparse, ssa, emit ]
    )
endforeach
