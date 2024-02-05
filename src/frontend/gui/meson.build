###
### imgui
###

gui_args = user_args + cpp.get_supported_arguments('/wd4611', '/wd4201', '-D_HAS_EXCEPTIONS=0')

src = [
    'imgui/imgui.cpp',
    'imgui/imgui_demo.cpp',
    'imgui/imgui_draw.cpp',
    'imgui/imgui_widgets.cpp',
    'imgui/imgui_tables.cpp',

    'imgui/backends/imgui_impl_win32.cpp',
    'imgui/backends/imgui_impl_dx12.cpp'
]

deps = [ cpp.find_library('d3d12'), cpp.find_library('dxgi') ]

libimgui = static_library('imgui', src,
    build_by_default : not meson.is_subproject(),
    cpp_args : gui_args,
    include_directories : [ 'imgui', 'imgui/backends' ],
    dependencies : [ deps, base ]
)

imgui = declare_dependency(
    link_with : libimgui,
    include_directories : [ 'imgui', 'imgui/backends' ],
    dependencies : [ deps, base ]
)

###
### gui
###

src = [
    'src/draw.cpp',
    'src/arena.cpp',
    'src/trace.cpp',
    'src/config.cpp',
    'src/panic.cpp',
    'src/sources.cpp',
    'src/compile.cpp',

    'main.cpp'
]

executable('gui', src,
    cpp_args : gui_args,
    build_by_default : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : [
        imgui, core, os, scan,
        memory, notify, config, setup,
        io, broker, support, ssa, tree, backtrace,
        arena
    ]
)
