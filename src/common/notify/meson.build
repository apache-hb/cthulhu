src = [
    'src/notify.c',
    'src/common.c',

    'src/text_rich.c',
    'src/text_simple.c',
    'src/text_stacktrace.c'
]

libnotify = library('notify', src,
    c_args : user_args,
    dependencies : [ scan, std, memory, io, format ],
    include_directories : 'include'
)

notify = declare_dependency(
    link_with : libnotify,
    dependencies : [ core, base, os, format ],
    include_directories : 'include'
)
