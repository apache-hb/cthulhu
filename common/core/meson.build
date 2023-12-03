config_header = configure_file(
    output : 'ctu-config.h',
    configuration : config_cdata
)

config_include = include_directories('.')

core = declare_dependency(
    include_directories : [ 'include', config_include ]
)
