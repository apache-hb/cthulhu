project('dear-imgui', 'cpp')

src = [
    'imgui/imgui_demo.cpp',
    'imgui/imgui_draw.cpp',
    'imgui/imgui_tables.cpp',
    'imgui/imgui_widgets.cpp',
    'imgui/imgui.cpp'
]

libimgui = library('imgui', src,
    include_directories : 'imgui'
)

imgui = declare_dependency(
    link_with : libimgui,
    include_directories : 'imgui'
)

backend = [
    'imgui/backends/imgui_impl_opengl3.cpp',
    'imgui/backends/imgui_impl_glfw.cpp'
]

glfw = dependency('glfw3', fallback : [ 'glfw', 'glfw3' ])

libbackend = library('imgui_backend', backend,
    include_directories : 'imgui/backends',
    dependencies : [ imgui, glfw ]
)

backend = declare_dependency(
    link_with : libbackend,
    include_directories : 'imgui/backends',
    dependencies : glfw
)
