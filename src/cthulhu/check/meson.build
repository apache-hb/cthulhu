check_api = configure_file(
    input : api_header,
    output : 'ctu_check_api.h',
    configuration : { 'MOD': 'CHECK' }
)

check_include = include_directories('.', 'include')

src = [
    'src/tree.c'
]

libcheck = library('check', src,
    build_by_default : false,
    c_args : user_args + [ '-DCT_CHECK_BUILD=1' ],
    include_directories : check_include,
    dependencies : [ arena, std, tree, ssa, scan, events, util ]
)

check = declare_dependency(
    link_with : libcheck,
    include_directories : check_include,
    dependencies : [ base ]
)
