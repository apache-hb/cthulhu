src = [
    'src/target.c',

    # tree output (TODO)
    'src/tree.c',

    # ssa output
    'src/common.c',
    'src/emit.c',
    'src/type.c'
]

# 'src/ssa.c',

libtarget_cfamily = static_library('target_cfamily', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : [ target, fs, io, events, std, tree, ssa ]
)

targets += {
    'cfamily': {
        'dep': declare_dependency(link_with : libtarget_cfamily),
        'mod': 'kTargetC',

        'static': libtarget_cfamily,
        'module': 'kTargetC'
    }
}
