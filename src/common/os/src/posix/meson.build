src = [ 'init.c', 'file.c', 'dir.c', 'fs.c', 'error.c' ]

platform = [ ]

lib_osnative = library('os_posix', src,
    c_args : user_args,
    dependencies : [ os_common, platform, base, std, memory ]
)
