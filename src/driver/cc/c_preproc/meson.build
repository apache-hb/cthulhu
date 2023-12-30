src = [
    'src/cpp.c',
    'src/scan.c',

    lex.process('src/cpp.l')
]

libpreproc = library('preproc', src,
    dependencies : [ core, base, scan, std, io, memory, interop, notify ],
    include_directories : [ 'include', 'src' ]
)

preproc = declare_dependency(
    link_with : libpreproc,
    include_directories : 'include'
)
