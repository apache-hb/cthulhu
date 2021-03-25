project('imgui', 'cpp')

src = [
    'imgui/imgui_demo.cpp',
    'imgui/imgui_draw.cpp',
    'imgui/imgui_tables.cpp',
    'imgui/imgui_widgets.cpp',
    'imgui/imgui.cpp',
    'imgui/backends/imgui_impl_dx11.cpp',
    'imgui/backends/imgui_impl_win32.cpp'
]

incs = include_directories('imgui', 'imgui/backends')

imgui = library('imgui', src,
    include_directories : incs
)

imgui_dep = declare_dependency(
    link_with : imgui,
    include_directories : incs
)