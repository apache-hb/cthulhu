
src = [
    'src/io.c',
    'src/common.c',
    'src/view.c',
    'src/buffer.c',
    'src/file.c',
    'src/console.c'
]

# TODO: make src/console.c optional for platforms without stdio.h

libio = library('io', src,
    c_args : user_args,
    dependencies : [ std, os, memory ],
    include_directories : [ 'include' ]
)

io = declare_dependency(
    link_with : libio,
    dependencies : [ os ],
    include_directories : 'include'
)
