src = [ 'init.c', 'file.c', 'dir.c', 'fs.c', 'error.c' ]

lib_osnative = library('os_windows', src,
    build_by_default : false,
    c_args : user_args,
    dependencies : [ arena, std, os_common ]
)
