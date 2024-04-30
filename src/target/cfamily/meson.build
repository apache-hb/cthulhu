src = [
    # tree output (TODO)
    'src/tree.c',

    # ssa output
    'src/common.c',
    'src/emit.c',
    'src/type.c'
]

# 'src/ssa.c',

deps = [ target, fs, io, events, std, tree, ssa ]

cfamily_target = static_library('target_cfamily', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : deps
)

cfamily = { }

if default_library == 'static'
    cfamily_static = static_library('cfamily_static', 'src/target.c',
        link_with : cfamily_target,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', cfamily_target.private_dir_include() ]
    )

    cfamily += {
        'dep': declare_dependency(link_with : cfamily_static),
        'mod': 'kTargetC',

        'static': cfamily_static,
        'module': 'kTargetC'
    }
elif default_library == 'shared'
    cfamily_shared = shared_library('cfamily_shared', 'src/target.c',
        link_with : cfamily_target,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', cfamily_target.private_dir_include() ]
    )

    cfamily += {
        'shared': cfamily_shared,
    }
endif

targets += {
    'cfamily': cfamily
}
