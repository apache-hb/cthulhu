src = [
    'src/memory.c',
    'src/arena.c'
]

libmemory = library('memory', src,
    c_args : user_args,
    dependencies : [ gmp, base ],
    include_directories : [ 'include' ]
)

memory = declare_dependency(
    link_with : libmemory,
    dependencies : [ gmp, core ],
    include_directories : [ 'include' ]
)
