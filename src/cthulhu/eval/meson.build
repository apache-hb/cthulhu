eval_api = configure_file(
    input : api_header,
    output : 'ctu_eval_api.h',
    configuration : { 'MOD': 'EVAL' }
)

eval_include = include_directories('.', 'include')

src = [ 'src/eval.c' ]

libeval = library('eval', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    c_args : [ '-DCT_EVAL_BUILD=1' ],
    dependencies : [ core ],
    include_directories : [ eval_include ]
)

eval = declare_dependency(
    link_with : libeval,
    include_directories : [ eval_include ]
)

install_headers(eval_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
