src = [
    'src/base.c',
    'src/panic.c',
    'src/endian.c',
    'src/memory.c'
]

libbase = library('base', src,
    c_args : user_args,
    dependencies : [ gmp, stacktrace ],
    include_directories : [ 'include', config_include ]
)

base = declare_dependency(
    link_with : libbase,
    dependencies : [ gmp, stacktrace ],
    include_directories : [ 'include', config_include ]
)
