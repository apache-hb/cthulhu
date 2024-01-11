src = [
    'src/util.c',
    'src/panic.c',
    'src/log.c',
    'src/endian.c'
]

libbase = library('base', src,
    c_args : user_args,
    dependencies : [ core ],
    include_directories : [ 'include' ]
)

base = declare_dependency(
    link_with : libbase,
    dependencies : [ core ],
    include_directories : [ 'include' ]
)
