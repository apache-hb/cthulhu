src = [
    'src/hlir/sema.c',
    'src/hlir/attribs.c',
    'src/hlir/check.c',
    
    'src/hlir/type.c',
    'src/hlir/hlir.c',
    'src/hlir/decl.c',
    'src/hlir/ops.c',
    'src/hlir/query.c',
    'src/hlir/common.c',
    'src/hlir/validate.c',
]

libgeneric = library('generic', src,
    include_directories : 'include',
    c_args : args,
    dependencies : [ std, gmp ]
)

generic = declare_dependency(
    link_with : libgeneric,
    include_directories : 'include',
    dependencies : [ gmp ]
)

src = [
    'src/ssa/ssa.c',
    'src/ssa/emit.c',
    'src/ssa/eval.c',
    'src/ssa/common.c'
]

libssa = library('ssa', src,
    dependencies : [ std, gmp ],
    c_args : args,
    include_directories : 'include'
)

ssa = declare_dependency(
    link_with : libssa,
    include_directories : 'include',
    dependencies : [ gmp ]
)

src = [
    'src/emit/c89.c',
    'src/emit/jvm.c',
    'src/emit/common.c'
]

libemit = library('emit', src,
    dependencies : [ ssa, std, io, interop ],
    c_args : args,
    include_directories : 'include'
)

emit = declare_dependency(
    link_with : libemit,
    include_directories : 'include',
    dependencies : [ ssa ]
)

src = [
    'src/mediator/interface.c',
    'src/mediator/driver.c'
]

libmediator = library('mediator', src,
    dependencies : [ std ],
    c_args : args,
    include_directories : 'include'
)

mediator = declare_dependency(
    link_with : libmediator,
    include_directories : [ 'include', versiondir ],
    dependencies : [ base ]
)
