src = [ 'common.c', 'file.c', 'dir.c', 'fs.c', 'error.c' ]

libnative = library('native', src,
    c_args : user_args,
    dependencies : [ base, common ]
)

native = declare_dependency(
    link_with : libnative,
    dependencies : [ base, std ]
)
