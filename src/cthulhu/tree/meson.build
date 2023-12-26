
src = [
    'src/tree.c',
    'src/ops.c',
    'src/sema.c',
    'src/decl.c',
    'src/query.c'
]

libtree = library('tree', src,
    include_directories : 'include',
    c_args : user_args,
    dependencies : [ memory, std, gmp, scan, events ]
)

tree = declare_dependency(
    link_with : libtree,
    include_directories : 'include',
    dependencies : [ gmp ]
)
