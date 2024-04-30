llvm = dependency('llvm', components : ['core'], method : 'config-tool', static : true)

deps = [
    target, fs, io, events, std, tree, ssa, base
]

src = [
    'src/ssa.c'
]

llvm_target = static_library('target_llvm', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : [ deps, llvm ]
)

llvm = { }

if default_library == 'static'
    llvm_static = static_library('llvm_static', 'src/target.c',
        link_with : llvm_target,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', llvm_target.private_dir_include() ]
    )

    llvm += {
        'dep': declare_dependency(link_with : llvm_static),
        'mod': 'kTargetLLVM',

        'static': llvm_static,
        'module': 'kTargetLLVM'
    }
elif default_library == 'shared'
    llvm_shared = shared_library('llvm_shared', 'src/target.c',
        link_with : llvm_target,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', llvm_target.private_dir_include() ]
    )

    llvm += {
        'shared': llvm_shared,
    }
endif

targets += {
    'llvm': llvm
}
