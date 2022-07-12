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

    # ssa intermediate
    'src/ssa/ssa.c',

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
    dependencies : deps,
    compile_args : args
)
