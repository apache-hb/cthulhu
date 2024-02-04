schema_api = configure_file(
    input : api_header,
    output : 'ctu_schema_api.h',
    configuration : { 'MOD': 'SCHEMA' }
)

schema_include = include_directories('.', 'include')

src = [ 'src/schema.c' ]

libschema = library('schema', src,
    build_by_default : not meson.is_subproject(),
    c_args : [ '-DCT_SCHEMA_BUILD=1' ],
    dependencies : [ core ],
    include_directories : schema_include
)

schema = declare_dependency(
    dependencies : [ base ],
    link_with : libschema,
    include_directories : schema_include
)

install_headers(schema_api, subdir : 'schema')
install_subdir('include', install_dir : 'include', strip_directory : true)
