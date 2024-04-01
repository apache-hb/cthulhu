src = [ 'src/target.c', 'src/ssa.c', 'src/tree.c', 'src/common.c' ]

deps = [ driver, events, std, broker, ssa, io, fs ]

# TODO: once we can load targets as shared libraries
# we need to do the same shared_module/static_library split
# that languages do
libtarget_debug = static_library('target_debug', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : deps
)

targets += {
    'debug': {
        'dep': declare_dependency(link_with : libtarget_debug),
        'mod': 'kTargetDebug',

        'static': libtarget_debug,
        'module': 'kTargetDebug'
    }
}
