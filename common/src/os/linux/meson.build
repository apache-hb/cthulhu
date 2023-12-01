src = [ 'common.c', 'file.c', 'dir.c', 'fs.c', 'error.c' ]

platform = [ cc.find_library('m'), cc.find_library('dl') ]

libnative = library('native', src,
    c_args : user_args,
    dependencies : [ common, platform ]
)

native = declare_dependency(
    link_with : libnative,
    dependencies : [ std, platform ]
)
