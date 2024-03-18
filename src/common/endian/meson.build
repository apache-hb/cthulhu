endian_api = configure_file(
    input : api_header,
    output : 'ctu_endian_api.h',
    configuration : { 'MOD': 'ENDIAN' }
)

inc = include_directories('.', 'include')
src = [ 'src/endian.c' ]

libendian = library('endian', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_ENDIAN_BUILD=1' ],
    dependencies : [ base ],
    include_directories : inc
)

endian = declare_dependency(
    link_with : libendian,
    dependencies : core,
    include_directories : inc
)

install_headers(endian_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
