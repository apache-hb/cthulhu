src = [
    'src/hlir/h2.c',
    'src/hlir/ops.c',
    'src/hlir/sema.c',
    'src/hlir/decl.c',
    'src/hlir/query.c'
]

libhlir = library('hlir', src,
    include_directories : 'include',
    c_args : args,
    dependencies : [ std, gmp ]
)

hlir = declare_dependency(
    link_with : libhlir,
    include_directories : 'include',
    dependencies : [ gmp ]
)

src = [
    'src/ssa/ssa.c',
    'src/ssa/opt.c',

    'src/ssa/common/type.c',
    'src/ssa/common/value.c',
    'src/ssa/common/operand.c'
]

libssa = library('ssa', src,
    include_directories : 'include',
    c_args : args,
    dependencies : [ std, gmp, hlir ]
)

ssa = declare_dependency(
    link_with : libssa,
    include_directories : 'include',
    dependencies : [ gmp, hlir ]
)

src = [
    'src/emit/common.c',
    'src/emit/ssa.c',

    'src/emit/c89/emit.c',
    'src/emit/c89/type.c'
]

libemit = library('emit', src,
    include_directories : [ 'include', 'src/emit' ],
    c_args : args,
    dependencies : [ ssa, std, io, interop ]
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
    include_directories : 'include',
    c_args : args,
    dependencies : [ std, hlir ]
)

mediator = declare_dependency(
    link_with : libmediator,
    include_directories : [ 'include', versiondir ],
    dependencies : [ base ]
)

src = [
    'src/util/util.c'
]

libutil = library('util', src,
    include_directories : 'include',
    c_args : args,
    dependencies : [ std, hlir ]
)

util = declare_dependency(
    link_with : libutil,
    include_directories : 'include',
    dependencies : [ base ]
)
