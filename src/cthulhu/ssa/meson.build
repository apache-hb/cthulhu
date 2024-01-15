ssa_api = configure_file(
    input : api_header,
    output : 'ctu_ssa_api.h',
    configuration : { 'MOD': 'SSA' }
)

ssa_include = include_directories('.', 'include')

src = [
    'src/ssa.c',
    'src/opt.c',

    'src/common/type.c',
    'src/common/value.c',
    'src/common/operand.c'
]

libssa = library('ssa', src,
    build_by_default : false,
    c_args : user_args + [ '-DCT_SSA_BUILD=1' ],
    dependencies : [ memory, std, tree, scan, events, arena ],
    include_directories : ssa_include
)

ssa = declare_dependency(
    link_with : libssa,
    include_directories : ssa_include,
    dependencies : [ tree ]
)
