schema_api = configure_file(
    input : api_header,
    output : 'ctu_schema_api.h',
    configuration : { 'MOD': 'SCHEMA' }
)

inc = include_directories('.', 'include')
src = [ 'src/schema.c' ]

libschema = library('schema', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_SCHEMA_BUILD=1' ],
    dependencies : [ arena ],
    include_directories : inc
)

schema = declare_dependency(
    link_with : libschema,
    dependencies : [ arena ],
    include_directories : inc
)

install_headers(schema_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
