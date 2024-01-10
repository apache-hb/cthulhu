src = [
    'src/main.c',
    'src/events.c',

    lex.process('src/pre.l'),
]

libcpp = library('pre', src,
    dependencies : [ runtime, interop, scan, notify, util, events ],
    c_args : generated_args,
    include_directories : [ 'src', 'include' ]
)

