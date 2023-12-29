src = [ 'main.c' ]

events_tool = executable('events_tool', src,
    dependencies : [
        defaults, argparse, config, io
    ]
)

build_events = generator(events_tool,
    output : [ '@BASENAME@_events.c', '@BASENAME@_events.h' ],
    arguments : [
        '--source=@OUTPUT0@',
        '--header=@OUTPUT1@',
        '--setup=@INPUT@'
    ]
)
