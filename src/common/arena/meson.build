src = [ 'src/arena.c' ]

libarena = library('arena', src,
    build_by_default : false,
    c_args : user_args,
    dependencies : [ gmp, base ],
    include_directories : [ 'include' ]
)

arena = declare_dependency(
    link_with : libarena,
    dependencies : [ gmp, core ],
    include_directories : [ 'include' ]
)
