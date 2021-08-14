src = [ 'scan.c' ]

libctu = library('cthulhu', src,
    dependencies : generic
)

options += declare_dependency(
    link_with : libctu,
    include_directories : '.',
    compile_args : '-DCTU_DRIVER=1'
)
