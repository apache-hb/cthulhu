gui_args = user_args + cpp.get_supported_arguments([
    '/wd4611', '/wd4201', '-D_HAS_EXCEPTIONS=0',
    '-Wno-deprecated-enum-enum-conversion', '-Wno-unused-function'
])

subdir('simcoe')

# we provide imgui here ourselves so we can configure the assert handler
subdir('imgui')

# we also modify imfilebrowser to not need exceptions enabled
subdir('imfilebrowser')
subdir('implot')

# this is just the d3d12 imgui example, but with a few changes
# to allow us to use it as a library.
# TODO: i should add more of the samples to support platforms
# other than windows.
subdir('draw')

src = [
    'src/arena.cpp',
    'src/trace.cpp',
    'src/config.cpp',
    'src/panic.cpp',
    'src/compile.cpp',

    'src/panels/panel.cpp',
    'src/panels/sources.cpp',
    'src/panels/info.cpp',
    'src/panels/arena.cpp',
    'src/panels/demo.cpp',

    'main.cpp'
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
