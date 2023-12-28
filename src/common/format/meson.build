src = [
    'src/colour.c',
    'src/config.c',
    'src/version.c'
]

libformat = library('format', src,
    c_args : user_args,
    dependencies : [ std, core, io, config ],
    include_directories : [ 'include' ]
)

format = declare_dependency(
    link_with : libformat,
    dependencies : [ std, core ],
    include_directories : [ 'include' ]
)
