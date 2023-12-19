src = [
    'src/events/events.c'
]

libevents = library('events', src,
    include_directories : 'include',
    c_args : user_args,
    dependencies : [ memory, std, scan, notify ]
)

events = declare_dependency(
    link_with : libevents,
    include_directories : 'include',
    dependencies : [ base, notify ]
)

src = [
    'src/tree/tree.c',
    'src/tree/ops.c',
    'src/tree/sema.c',
    'src/tree/decl.c',
    'src/tree/query.c'
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

src = [
    'src/ssa/ssa.c',
    'src/ssa/opt.c',

    'src/ssa/common/type.c',
    'src/ssa/common/value.c',
    'src/ssa/common/operand.c'
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

src = [
    'src/emit/common.c',
    'src/emit/ssa.c',

    'src/emit/c89/emit.c',
    'src/emit/c89/type.c'
]

libemit = library('emit', src,
    include_directories : [ 'include', 'src/emit' ],
    c_args : user_args,
    dependencies : [ memory, base, ssa, std, io, fs, scan, events, interop ]
)

emit = declare_dependency(
    link_with : libemit,
    include_directories : 'include',
    dependencies : [ base, ssa ]
)

src = [
    'src/mediator/interface.c',
    'src/mediator/driver.c'
]

libmediator = library('mediator', src,
    include_directories : 'include',
    c_args : user_args,
    dependencies : [ memory, base, std, stacktrace, scan, events, tree, interop ]
)

mediator = declare_dependency(
    link_with : libmediator,
    include_directories : [ 'include' ],
    dependencies : [ base, std ]
)

src = [
    'src/util/util.c',
    'src/util/text.c',
    'src/util/query.c'
]

libutil = library('util', src,
    include_directories : 'include',
    c_args : user_args,
    dependencies : [ memory, base, std, scan, events, tree ]
)

util = declare_dependency(
    link_with : libutil,
    include_directories : 'include',
    dependencies : [ base ]
)

src = [
    'src/check/tree.c'
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
