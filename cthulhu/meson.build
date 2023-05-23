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
    'src/emit/c89-hlir.c',
    'src/emit/c89-ssa.c',

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
    'src/mediator/mediator.c',
    'src/mediator/language.c',
    'src/mediator/plugin.c',
    'src/mediator/lifetime.c'
]

libmediate = library('mediate', src,
    dependencies : [ std, emit ],
    c_args : args,
    include_directories : 'include'
)

mediate = declare_dependency(
    link_with : libmediate,
    include_directories : [ 'include', versiondir ],
    dependencies : [ base ]
)
