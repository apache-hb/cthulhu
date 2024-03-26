src = [
    'src/arena.cpp',
    'src/config.cpp',
    'src/panic.cpp',
    'src/compile.cpp',
    'src/utils.cpp',

    'src/panels/panel.cpp',
    'src/panels/sources.cpp',
    'src/panels/info.cpp',
    'src/panels/arena.cpp',
    'src/panels/demo.cpp',
    'src/panels/settings.cpp',

    'src/main.cpp'
]

executable('gui', src,
    cpp_args : gui_args,
    cpp_pch : 'src/stdafx.hpp',
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : [ 'include', 'src' ],
    dependencies : [
        draw, imgui, imfilebrowser, implot,
        core, os, scan,
        memory, notify, config, setup,
        io, broker, support, ssa, tree, backtrace,
        arena, interop
    ]
)