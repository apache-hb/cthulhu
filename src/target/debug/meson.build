src = [ 'src/target.c' ]

# TODO: once we can load targets as shared libraries
# we need to do the same shared_module/static_library split
# that languages do
libtarget_debug = static_library('target_debug', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : [ target ]
)

targets += {
    'debug': {
        'dep': declare_dependency(link_with : libtarget_debug),
        'mod': 'kTargetDebug'
    }
}
