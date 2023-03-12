glad = subproject('glad').get_variable('glad')

src = [
    'interface/imgui/imgui_demo.cpp',
    'interface/imgui/imgui_draw.cpp',
    'interface/imgui/imgui_tables.cpp',
    'interface/imgui/imgui_widgets.cpp',
    'interface/imgui/imgui.cpp',

    'interface/imgui/misc/cpp/imgui_stdlib.cpp'
]

libimgui = library('imgui', src,
    include_directories : 'interface/imgui',
    dependencies : [ base ]
)

imgui = declare_dependency(
    link_with : libimgui,
    include_directories : 'interface/imgui',
    dependencies : [ base ]
)

src = [
    'interface/imgui/backends/imgui_impl_opengl3.cpp',
    'interface/imgui/backends/imgui_impl_glfw.cpp'
]

glfw = dependency('glfw3', fallback : [ 'glfw', 'glfw3' ])

libbackend = library('imgui_backend', src,
    include_directories : 'interface/imgui/backends',
    dependencies : [ imgui, glfw ]
)

backend = declare_dependency(
    link_with : libbackend,
    include_directories : 'interface/imgui/backends',
    dependencies : glfw
)

src = [ 
    'interface/imnodes/ImNodes.cpp',
    'interface/imnodes/ImNodesEz.cpp',
]

libimnodes = library('imnodes', src,
    dependencies : [ imgui ],
    include_directories : 'interface/imnodes'
)

imnodes = declare_dependency(
    include_directories : 'interface',
    link_with : libimnodes,
    dependencies : [ imgui ]
)

foreach name, lib : langs
    executable(name, 'interface/main.cpp',
        dependencies : [ lib, generic, imgui, imnodes, backend, glfw, glad ]
    )
endforeach
