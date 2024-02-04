src = [ 'src/target.c' ]

libtarget_cfamily = library('target_cfamily', src,
    build_by_default : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : [ target ]
)

targets += {
    'cfamily': {
        'dep': declare_dependency(link_with : libtarget_cfamily),
    }
}
