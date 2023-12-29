src = [
    'src/cpp.c'
]

libpreproc = library('preproc', src,
    dependencies : [ core, base, scan, std, io, memory ],
    include_directories : 'include'
)

preproc = declare_dependency(
    link_with : libpreproc,
    include_directories : 'include'
)
