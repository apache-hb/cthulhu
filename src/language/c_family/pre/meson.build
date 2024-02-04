src = [
    'src/main.c',
    'src/events.c',
    'src/scan.c',

    lex.process('src/pre.l'),
]

libcpp = library('pre', src,
    dependencies : [
        broker, interop,
        scan, notify, util,
        events, driver
    ],
    c_args : generated_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ]
)
