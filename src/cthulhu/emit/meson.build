emit_api = configure_file(
    input : api_header,
    output : 'ctu_emit_api.h',
    configuration : { 'MOD': 'EMIT' }
)

emit_include = include_directories('.', 'include')
emit_impl_include = include_directories('src')

src = [
    'src/common.c',
    'src/ssa.c',

    'src/c89/emit.c',
    'src/c89/type.c'
]

libemit = library('emit', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_EMIT_BUILD=1' ],
    include_directories : [ emit_impl_include, emit_include ],
    dependencies : [ arena, base, ssa, std, io, fs, scan, events, interop ]
)

emit = declare_dependency(
    link_with : libemit,
    include_directories : emit_include,
    dependencies : [ base, ssa ]
)

install_headers(emit_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
