src = [
    'src/arena.cpp',
    'src/config.cpp',
    'src/panic.cpp',
    'src/compile.cpp',
    'src/editor.cpp',

    'src/panels/panel.cpp',
    'src/panels/sources.cpp',
    'src/panels/info.cpp',
    'src/panels/arena.cpp',
    'src/panels/demo.cpp',
    'src/panels/theme.cpp',
    'src/panels/json.cpp',

    'src/utils/io.cpp',

    'src/main.cpp'
]

flecs = subproject('flecs').get_variable('flecs_dep')

deps = [
    # render backend
    draw,

    # imgui deps and widgets
    imgui, imfilebrowser, implot, memedit,

    # cthulhu common deps
    core, os, scan, io, arena, interop, backtrace,
    notify, config,

    # cthulhu runtime deps
    setup, memory, broker, support, ssa, tree,

    # extra utilities
    json,

    # ecs
    flecs
]

executable('gui', src,
    cpp_args : gui_args,
    cpp_pch : 'src/stdafx.hpp',
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : [ 'include', 'src' ],
    dependencies : deps
)
