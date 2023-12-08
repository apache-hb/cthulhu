###
### imgui
###

cpp = meson.get_compiler('cpp')

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

libimgui = library('imgui', src,
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
    'src/config.cpp',

    'main.cpp'
]

executable('gui', src,
    include_directories : 'include',
    dependencies : [ imgui, core, os, memory, config ]
)
