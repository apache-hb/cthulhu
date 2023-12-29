
src = [
    'src/argparse.c',
    'src/common.c'
]

libargparse_base = library('argparse_base', src,
    c_args : user_args,
    dependencies : [ gmp, std, scan, interop, config ],
    include_directories : [ 'src', 'include' ]
)

src = [
    'src/parse.c',
    lex.process('src/ap.l'),
    parse.process('src/ap.y')
]

libargparse = library('argparse', src,
    c_args : generated_args,
    link_with : libargparse_base,
    dependencies : [ gmp, memory, std, scan, interop, io, config ],
    include_directories : [ 'src', 'include' ]
)

argparse = declare_dependency(
    link_with : [ libargparse_base, libargparse ],
    include_directories : 'include',
    dependencies : [ gmp, core ]
)