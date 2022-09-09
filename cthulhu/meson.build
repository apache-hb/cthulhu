src = [
    'src/hlir/sema.c',
    'src/hlir/attribs.c',
    
    'src/hlir/type.c',
    'src/hlir/hlir.c',
    'src/hlir/decl.c',
    'src/hlir/ops.c',
    'src/hlir/query.c',
    'src/hlir/common.c',
    'src/hlir/validate.c',

    # common frontend components
    'src/interface/runtime.c',
    'src/interface/cthulhu.c',

    # tree output
    'src/emit/c89.c'
]

libgeneric = library('generic', src,
    include_directories : 'include',
    c_args : args,
    dependencies : deps
)

generic = declare_dependency(
    link_with : libgeneric,
    include_directories : 'include',
    dependencies : deps
)

src = [
    'src/ssa/ssa.c',
    'src/ssa/eval.c'
]

libssa = library('ssa', src,
    dependencies : deps,
    c_args : args,
    include_directories : 'include'
)

ssa = declare_dependency(
    link_with : libssa,
    include_directories : 'include',
    dependencies : deps,
)
