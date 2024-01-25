src = [ 'init.c', 'file.c', 'dir.c', 'fs.c', 'error.c', 'library.c' ]

# we use a switch case over bitflags in get_protect
# this is safe, but compilers warn about it
osnative_args = cc.get_supported_arguments('-Wno-switch', '/wd4063')

lib_osnative = library('os_windows', src,
    build_by_default : false,
    c_args : user_args + osnative_args,
    dependencies : [ arena, std, os_common ]
)
