src = [
    'src/colour.c',
    'src/config.c',
    'src/version.c',

    'src/common.c',
    'src/text_rich.c',
    'src/text_simple.c',
    'src/backtrace.c',
    'src/notify.c'
]

libformat = library('format', src,
    c_args : user_args,
    dependencies : [ std, core, io, scan, notify, config, backtrace, memory ],
    include_directories : [ 'include' ]
)

format = declare_dependency(
    link_with : libformat,
    dependencies : [ std, core, notify ],
    include_directories : [ 'include' ]
)
