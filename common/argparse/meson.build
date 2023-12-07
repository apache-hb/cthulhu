
src = [
    'src/argparse.c',
    'src/common.c',
    'src/commands.c',
]

libargparse_base = library('argparse_base', src,
    c_args : user_args,
    dependencies : [ gmp, std, scan, report, interop ],
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
    dependencies : [ gmp, memory, std, scan, report, interop, io ],
    include_directories : [ 'src', 'include' ]
)

argparse = declare_dependency(
    link_with : [ libargparse_base, libargparse ],
    include_directories : 'include',
    dependencies : [ gmp, core ]
)
