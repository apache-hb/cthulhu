gui_args = user_args + cpp.get_supported_arguments('/wd4611', '/wd4201', '-D_HAS_EXCEPTIONS=0')

src = [
    'imgui.cpp',
    'imgui_demo.cpp',
    'imgui_draw.cpp',
    'imgui_widgets.cpp',
    'imgui_tables.cpp',

    'backends/imgui_impl_win32.cpp',
    'backends/imgui_impl_dx12.cpp',

    'misc/cpp/imgui_stdlib.cpp',
]

deps = [ cpp.find_library('d3d12'), cpp.find_library('dxgi'), base ]
inc = include_directories('.', 'misc/cpp')

libimgui = static_library('imgui', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    cpp_args : gui_args,
    include_directories : inc,
    dependencies : deps
)

imgui = declare_dependency(
    link_with : libimgui,
    include_directories : inc,
    dependencies : deps
)
