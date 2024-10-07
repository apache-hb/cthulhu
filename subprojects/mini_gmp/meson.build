project('mini_gmp', 'c',
    default_options : [
        'warning_level=1',
        'werror=false'
    ],
    version : '6.0.0',
    license : 'LGPLv3',
    license_files : [ 'LICENSE' ],
    meson_version : '>=1.3.0',
)

cc = meson.get_compiler('c')
c_args = cc.get_supported_arguments('/wd4146')

is_shared = get_option('default_library') == 'shared'

export_args = is_shared ? [ '-DMINI_GMP_SHARED=1' ] : []

libgmp = library('gmp', [ 'mini-gmp.c', 'mini-mpq.c' ],
    build_by_default : false,
    install : true,
    include_directories : '.',
    c_args : c_args,
    c_shared_args : export_args + [ '-DMINI_GMP_EXPORTS=1' ]
)

gmp = declare_dependency(
    link_with : libgmp,
    include_directories : '.',
    compile_args : export_args
)
