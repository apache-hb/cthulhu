ctu_config_header = configure_file(
    output : 'ctu_config.h',
    configuration : config_cdata
)

inc = include_directories('.')

core = declare_dependency(
    include_directories : [ 'include', inc ]
)

# use files() as this file is referenced in other places and we need to preserve the path properly
api_header = files('data/module_api.h.in')

install_headers(ctu_config_header)
install_subdir('include', install_dir : 'include', strip_directory : true)
