src = [ 'common.c', 'file.c', 'dir.c', 'fs.c', 'error.c' ]

platform = [ cc.find_library('m'), cc.find_library('dl') ]

lib_osnative = library('os_linux', src,
    c_args : user_args,
    dependencies : [ os_common, platform, base, std, memory ]
)
