toml_api = configure_file(
    input : api_header,
    output : 'ctu_toml_api.h',
    configuration : { 'MOD': 'TOML' }
)

toml_include = include_directories('.', 'include')

src = [
    'src/toml.c',
    lex.process('src/toml.l'),
    parse.process('src/toml.y')
]

libtoml = library('toml', src,
    build_by_default : false,
    c_args : [ '-DCT_TOML_BUILD=1' ],
    dependencies : [ core, scan ],
    include_directories : toml_include
)

toml = declare_dependency(
    link_with : libtoml,
    include_directories : toml_include,
    dependencies : [ base ]
)

install_headers(toml_api, subdir : 'toml')
install_subdir('include', install_dir : 'include', strip_directory : true)
