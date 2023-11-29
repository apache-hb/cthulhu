foreach name, lib : langs
    exec = executable(name, 'interface/main.c',
        dependencies : [ lib, argparse, ssa, emit ]
    )
endforeach
