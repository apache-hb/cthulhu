# src = [
#     'src/hlir/sema.c',
#     'src/hlir/attribs.c',
#     'src/hlir/check.c',
    
#     'src/hlir/type.c',
#     'src/hlir/hlir.c',
#     'src/hlir/decl.c',
#     'src/hlir/ops.c',
#     'src/hlir/query.c',
#     'src/hlir/common.c',
#     'src/hlir/validate.c',
# ]

# libgeneric = library('generic', src,
#     include_directories : 'include',
#     c_args : args,
#     dependencies : [ std, gmp ]
# )

# generic = declare_dependency(
#     link_with : libgeneric,
#     include_directories : 'include',
#     dependencies : [ gmp ]
# )

src = [
    'src/hlir2/h2.c',
    'src/hlir2/check.c',
    'src/hlir2/sema.c',
    'src/hlir2/decl.c',
    'src/hlir2/query.c'
]

libh2 = library('hlir2', src,
    include_directories : 'include',
    c_args : args,
    dependencies : [ std, gmp ]
)

h2 = declare_dependency(
    link_with : libh2,
    include_directories : 'include',
    dependencies : [ gmp ]
)

# src = [
#     'src/ssa/ssa.c',
#     'src/ssa/emit.c',
#     'src/ssa/eval.c',
#     'src/ssa/common.c'
# ]

# libssa = library('ssa', src,
#     include_directories : 'include',
#     c_args : args,
#     dependencies : [ std, gmp, h2 ]
# )

# ssa = declare_dependency(
#     link_with : libssa,
#     include_directories : 'include',
#     dependencies : [ gmp, h2 ]
# )

# src = [
#     'src/emit/c89.c',
#     'src/emit/jvm.c',
#     'src/emit/common.c'
# ]

# libemit = library('emit', src,
#     include_directories : 'include',
#     c_args : args,
#     dependencies : [ ssa, std, io, interop ]
# )

# emit = declare_dependency(
#     link_with : libemit,
#     include_directories : 'include',
#     dependencies : [ ssa ]
# )

src = [
    'src/mediator/interface.c',
    'src/mediator/driver.c'
]

libmediator = library('mediator', src,
    include_directories : 'include',
    c_args : args,
    dependencies : [ std, h2 ]
)

mediator = declare_dependency(
    link_with : libmediator,
    include_directories : [ 'include', versiondir ],
    dependencies : [ base, h2 ]
)
