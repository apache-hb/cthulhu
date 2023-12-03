
src = [
    'src/io.c',
    'src/common.c',
    'src/view.c',
    'src/buffer.c',
    'src/file.c'
]

libio = library('io', src,
    c_args : user_args,
    dependencies : [ std, os ],
    include_directories : [ 'include' ]
)

io = declare_dependency(
    link_with : libio,
    dependencies : [ std, os ],
    include_directories : 'include'
)
