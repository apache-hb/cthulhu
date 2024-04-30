src = [ 'src/ssa.c', 'src/tree.c', 'src/common.c' ]

deps = [ driver, events, std, broker, ssa, io, fs ]

dbg_target = static_library('target_debug', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : deps
)

dbg = { }

if default_library == 'static'
    dbg_static = static_library('debug_static', 'src/target.c',
        link_with : dbg_target,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', dbg_target.private_dir_include() ]
    )

    dbg += {
        'dep': declare_dependency(link_with : dbg_static),
        'mod': 'kTargetDebug',

        'static': dbg_static,
        'module': 'kTargetDebug'
    }
elif default_library == 'shared'
    dbg_shared = shared_library('debug_shared', 'src/target.c',
        link_with : dbg_target,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', dbg_target.private_dir_include() ]
    )

    dbg += {
        'shared': dbg_shared,
    }
endif

targets += {
    'debug': dbg
}
