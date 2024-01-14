src = [
    'src/memory.c'
]

libmemory = library('memory', src,
    build_by_default : false,
    c_args : user_args,
    dependencies : [ gmp, arena, base ],
    include_directories : [ 'include' ]
)

memory = declare_dependency(
    link_with : libmemory,
    dependencies : [ gmp, arena ],
    include_directories : [ 'include' ]
)
