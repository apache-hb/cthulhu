src = [
    'src/ssa.c',
    'src/target.c'
]

libtarget_llvm = static_library('target_llvm', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : [ target, fs, io, events, std, tree ]
)

targets += {
    'llvm': {
        'dep': declare_dependency(link_with : libtarget_llvm),
        'mod': 'kTargetLLVM',

        'static': libtarget_llvm,
        'module': 'kTargetLLVM'
    }
}
