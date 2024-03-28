src = [
    'src/tree.c',
    'src/target.c'
]

libtarget_cfamily = static_library('target_cfamily', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : [ target, fs, io, events ]
)

targets += {
    'cfamily': {
        'dep': declare_dependency(link_with : libtarget_cfamily),
        'mod': 'kTargetC'
    }
}
