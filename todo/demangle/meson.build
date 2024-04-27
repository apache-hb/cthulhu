demangle_api = configure_file(
    input : api_header,
    output : 'ctu_demangle_api.h',
    configuration : { 'MOD': 'DEMANGLE' }
)

demangle_include = include_directories('.', 'include')

src = [
    'src/itanium.c',
    'src/microsoft.c'
]

libdemangle = library('demangle', src,
    build_by_default : not meson.is_subproject(),
    c_args : [ '-DCT_DEMANGLE_BUILD=1' ],
    dependencies : [ base, arena, std ],
    include_directories : demangle_include
)

demangle = declare_dependency(
    dependencies : [ core ],
    link_with : libdemangle,
    include_directories : demangle_include
)

install_headers(demangle_api, subdir : 'demangle')
install_subdir('include', install_dir : 'include', strip_directory : true)

demangle_tool = executable('ctu-demangle', 'tool/tool.c',
    dependencies : [ demangle, setup, memory ]
)
