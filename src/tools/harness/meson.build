src = [ 'main.c' ]

harness = executable('harness', src,
    export_dynamic : module_loader.contains('shared'),
    dependencies : [
        broker, argparse, support,
        ssa, emit, check, io, fs, setup,
        scan, notify, memory, events, format,
        arena
    ]
)
