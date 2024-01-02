src = [
    'src/cpp.c',
    'src/scan.c',
    'src/ast.c',
    'src/expand.c',

    lex.process('src/cpp.l'),
    parse.process('src/cpp.y')
]

libpreproc = library('preproc', src,
    dependencies : [ core, base, scan, std, io, memory, interop, notify, events, tree ],
    include_directories : [ 'include', 'src' ],
    c_args : generated_args
)

preproc = declare_dependency(
    link_with : libpreproc,
    include_directories : 'include'
)
