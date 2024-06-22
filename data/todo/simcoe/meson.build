simcoe_api = configure_file(
    input : api_header,
    output : 'ctu_simcoe_api.h',
    configuration : { 'MOD': 'SIMCOE' }
)

inc = include_directories('.', 'include')
src = [ 'src/units.cpp' ]

libsimcoe = library('simcoe', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    cpp_args : gui_args + [ '-DCT_SIMCOE_BUILD=1' ],
    include_directories : inc,
    dependencies : [ base, std ]
)

simcoe = declare_dependency(
    link_with : libsimcoe,
    include_directories : inc,
    dependencies : core
)

install_headers(simcoe_api)
install_subdir('include', install_dir : 'include', strip_directory : true)

executable('get_max_size', 'main.cpp',
    dependencies : simcoe
)
