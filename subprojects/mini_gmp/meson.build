project('mini_gmp', 'c',
    version : '6.0.0',
    license : 'LGPLv3',
    default_options : [
        'warning_level=1',
        'werror=false'
    ]
)

cc = meson.get_compiler('c')
c_args = cc.get_supported_arguments('/wd4146')

is_shared = get_option('default_library') == 'shared'

export_args = []
if is_shared
    export_args += [ '-DMINI_GMP_SHARED=1' ]
endif

libgmp = library('gmp', [ 'mini_gmp.c', 'mini_mpq.c' ],
    build_by_default : false,
    include_directories : '.',
    c_args : c_args,
    c_shared_args : export_args + [ '-DMINI_GMP_EXPORTS=1' ]
)

gmp = declare_dependency(
    link_with : libgmp,
    include_directories : '.',
    compile_args : export_args
)
