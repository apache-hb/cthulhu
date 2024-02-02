argparse_api = configure_file(
    input : api_header,
    output : 'ctu_argparse_api.h',
    configuration : { 'MOD': 'ARGPARSE' }
)

argparse_include = include_directories('.', 'include')
argparse_impl_include = include_directories('src')

# TODO: restructure this to expose less implementation details

# src = [
#     'src/argparse.c',
#     'src/common.c'
# ]

# libargparse_base = library('argparse_base', src,
#     c_args : user_args + [ '-DCT_ARGPARSE_BUILD=1' ],
#     dependencies : [ gmp, std, scan, io, interop, config ],
#     include_directories : [ argparse_impl_include, argparse_include ]
# )

src = [
    'src/argparse.c',
    'src/common.c',
    'src/parse.c',
    lex.process('src/ap.l'),
    parse.process('src/ap.y')
]

libargparse = library('argparse', src,
    build_by_default : false,
    c_args : generated_args + [ '-DCT_ARGPARSE_BUILD=1' ],
    dependencies : [ gmp, memory, std, scan, interop, io, config ],
    include_directories : [ argparse_impl_include, argparse_include ],
    override_options : [ 'unity=off' ]
)

argparse = declare_dependency(
    link_with : [ libargparse ],
    include_directories : argparse_include,
    dependencies : [ core ]
)
