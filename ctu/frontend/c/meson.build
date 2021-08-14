src = [ 'scan.c' ]

libcc = library('c', src,
    dependencies : generic
)

options += declare_dependency(
    link_with : libcc,
    include_directories : '.',
    compile_args : '-DC_DRIVER=1'
)
