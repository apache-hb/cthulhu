src = [
    'src/util.c',
    'src/panic.c',
    'src/endian.c'
]

libbase = library('base', src,
    c_args : user_args,
    dependencies : [ stacktrace ],
    include_directories : [ 'include' ]
)

base = declare_dependency(
    link_with : libbase,
    dependencies : [ stacktrace ],
    include_directories : [ 'include' ]
)
