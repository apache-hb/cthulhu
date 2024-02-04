tree_api = configure_file(
    input : api_header,
    output : 'ctu_tree_api.h',
    configuration : { 'MOD': 'TREE' }
)

tree_include = include_directories('.', 'include')

src = [
    'src/context.c',
    'src/tree.c',
    'src/ops.c',
    'src/sema.c',
    'src/decl.c',
    'src/query.c'
]

libtree = library('tree', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_TREE_BUILD=1' ],
    include_directories : tree_include,
    dependencies : [ memory, std, gmp, scan, events, arena ]
)

tree = declare_dependency(
    link_with : libtree,
    include_directories : tree_include,
    dependencies : [ gmp, std ] # export std due to bitset
)

install_headers(tree_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
