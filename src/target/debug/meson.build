src = [ 'src/target.c' ]

libtarget_debug = library('target_debug', src,
    build_by_default : false,
    include_directories : 'include',
    dependencies : [ target ]
)

targets += {
    'debug': {
        'dep': declare_dependency(link_with : libtarget_debug),
    }
}