src = [ 'src/cmd.c', 'main.c' ]

cli = executable('cli', src,
    build_by_default : false,
    include_directories : [ 'include' ],
    link_args : setargv,
    dependencies : [
        memory, runtime, argparse, support,
        tree, ssa, emit, io, fs,
        scan, notify, config, events, setup, format,
        arena, check
    ]
)

codegen = generator(cli,
    output : [ '@BASENAME@.reflect.c', '@BASENAME@.reflect.h' ],
    arguments : [
        '@INPUT@',
        '--source=@OUTPUT0@',
        '--header=@OUTPUT1@',
        '--reflect'
    ]
)

reflect = generator(cli,
    output : [ '@BASENAME@.reflect.h' ],
    arguments : [
        '@INPUT@',
        '--source=@OUTPUT0@.discard.c',
        '--header=@OUTPUT0@',
        '--reflect'
    ]
)
