headers = [
    'include/editor/panels/arena.hpp',
    'include/editor/panels/info.hpp',
    'include/editor/panels/panel.hpp',
    'include/editor/panels/sources.hpp',

    'include/editor/utils/io.hpp',
    'include/editor/utils/utils.hpp',

    'include/editor/arena.hpp',
    'include/editor/compile.hpp',
    'include/editor/config.hpp',
    'include/editor/memory.hpp',
    'include/editor/panic.hpp',
    'include/editor/utils.hpp',
]

src = [
    'src/arena.cpp',
    'src/config.cpp',
    'src/panic.cpp',
    'src/compile.cpp',
    'src/memory.cpp',

    'src/panels/panel.cpp',
    'src/panels/sources.cpp',
    'src/panels/info.cpp',
    'src/panels/arena.cpp',

    'src/utils/io.cpp',

    'src/main.cpp'
]

deps = [
    # render backend
    draw,

    # imgui deps and widgets
    imgui, imfilebrowser, implot, memedit,

    # cthulhu common deps
    core, os, scan, io, arena, interop, backtrace,
    notify, config, format,

    # cthulhu runtime deps
    setup, memory, broker, support, ssa, tree,

    # extra utilities
    json,

    # threading
    dependency('concurrentqueue')
]

executable('gui', src,
    cpp_args : gui_args,
    cpp_pch : 'src/stdafx.hpp',
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : [ 'include', 'src' ],
    dependencies : deps,
    extra_files : headers
)
