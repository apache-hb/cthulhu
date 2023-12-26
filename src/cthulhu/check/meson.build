
src = [
    'src/tree.c'
]

libcheck = library('check', src,
    include_directories : 'include',
    c_args : user_args,
    dependencies : [ memory, std, tree, ssa, scan, events, util ]
)

check = declare_dependency(
    link_with : libcheck,
    include_directories : 'include',
    dependencies : [ base ]
)
