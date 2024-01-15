config_header = configure_file(
    output : 'ctu_config.h',
    configuration : config_cdata
)

ctu_config_include = include_directories('.')

core = declare_dependency(
    include_directories : [ 'include', ctu_config_include ]
)

# use files() as this file is referenced in other places
# and files() preserves the path properly
api_header = files('data/module_api.h.in')
