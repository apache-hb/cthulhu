driver_api = configure_file(
    input : api_header,
    output : 'ctu_driver_api.h',
    configuration : { 'MOD': 'DRIVER' }
)

driver_include = include_directories('.', 'include')

driver = declare_dependency(
    compile_args : [ '-DCT_DRIVER_BUILD=1' ],
    include_directories : [ driver_include ]
)
