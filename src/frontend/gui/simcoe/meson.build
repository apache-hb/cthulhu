simcoe_api = configure_file(
    input : api_header,
    output : 'ctu_simcoe_api.h',
    configuration : { 'MOD': 'SIMCOE' }
)

inc = include_directories('.', 'include')
src = [ 'src/units.cpp' ]
deps = [ base ]

libsimcoe = library('simcoe', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    cpp_args : gui_args + [ '-DCT_SIMCOE_BUILD=1' ],
    include_directories : inc,
    dependencies : deps
)

simcoe = declare_dependency(
    link_with : libsimcoe,
    include_directories : inc,
    dependencies : deps
)

install_headers(simcoe_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
