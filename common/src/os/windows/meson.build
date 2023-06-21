src = [ 'common.c', 'dir.c', 'fs.c' ]

libnative = library('native', src,
    c_args : args,
    dependencies : [ common ]
)

native = declare_dependency(
    link_with : libnative,
    dependencies : [ std ]
)
