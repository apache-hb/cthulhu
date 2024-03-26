src = [ 'init.c', 'file.c', 'dir.c', 'fs.c', 'error.c', 'library.c' ]

lib_osnative = static_library('os_windows', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + os_args + [ os_api_build ],
    dependencies : [ arena, std, os_common ]
)
