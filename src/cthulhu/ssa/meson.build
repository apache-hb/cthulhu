
src = [
    'src/ssa.c',
    'src/opt.c',

    'src/common/type.c',
    'src/common/value.c',
    'src/common/operand.c'
]

libssa = library('ssa', src,
    include_directories : 'include',
    c_args : user_args,
    dependencies : [ memory, std, tree, scan, events ]
)

ssa = declare_dependency(
    link_with : libssa,
    include_directories : 'include',
    dependencies : [ tree ]
)
