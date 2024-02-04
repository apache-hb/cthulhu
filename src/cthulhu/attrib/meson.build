attrib_api = configure_file(
    input : api_header,
    output : 'ctu_attrib_api.h',
    configuration : { 'MOD': 'ATTRIB' }
)

attrib_include = include_directories('.', 'include')

src = [ 'src/attrib.c' ]

libattrib = library('attrib', src,
    build_by_default : not meson.is_subproject(),
    c_args : [ '-DCT_ATTRIB_BUILD=1' ],
    dependencies : [ core, tree ],
    include_directories : attrib_include
)

attrib = declare_dependency(
    dependencies : [ core ],
    link_with : libattrib,
    include_directories : attrib_include
)

install_headers(attrib_api, subdir : 'attrib')
install_subdir('include', install_dir : 'include', strip_directory : true)
