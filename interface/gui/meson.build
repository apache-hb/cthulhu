glad = subproject('glad').get_variable('glad')

src = [
    'interface/include/imgui/imgui_demo.cpp',
    'interface/include/imgui/imgui_draw.cpp',
    'interface/include/imgui/imgui_tables.cpp',
    'interface/include/imgui/imgui_widgets.cpp',
    'interface/include/imgui/imgui.cpp',

    'interface/include/imgui/misc/cpp/imgui_stdlib.cpp'
]

libimgui = library('imgui', src,
    include_directories : 'interface/include/imgui',
    dependencies : [ base ]
)

imgui = declare_dependency(
    link_with : libimgui,
    include_directories : 'interface/include/imgui',
    dependencies : [ base ]
)

src = [
    'interface/include/imgui/backends/imgui_impl_opengl3.cpp',
    'interface/include/imgui/backends/imgui_impl_glfw.cpp'
]

glfw = dependency('glfw3', fallback : [ 'glfw', 'glfw3' ])

libbackend = library('imgui_backend', src,
    include_directories : 'interface/include/imgui/backends',
    dependencies : [ imgui, glfw ]
)

backend = declare_dependency(
    link_with : libbackend,
    include_directories : 'interface/include/imgui/backends',
    dependencies : glfw
)

src = [
    'interface/include/imnodes/ImNodes.cpp',
    'interface/include/imnodes/ImNodesEz.cpp',
]

libimnodes = library('imnodes', src,
    dependencies : [ imgui ],
    include_directories : 'interface/include/imnodes'
)

imnodes = declare_dependency(
    include_directories : 'interface/include',
    link_with : libimnodes,
    dependencies : [ imgui ]
)

src = [
    'interface/include/file-dialog/ImGuiFileDialog.cpp'
]

libdialog = library('file-dialog', src,
    dependencies : [ imgui ],
    include_directories : 'interface/include/file-dialog'
)

dialog = declare_dependency(
    include_directories : 'interface',
    link_with : libdialog,
    dependencies : [ imgui ]
)

libeditor = library('editor', 'interface/src/editor.cpp',
    dependencies : [ imgui, imnodes, dialog, backend, glfw, glad ],
    include_directories : 'interface'
)

editor = declare_dependency(
    link_with : libeditor,
    dependencies : [ imgui, imnodes, dialog, backend, glfw, glad ]
)

error('gui not implemented')

# foreach name, lib : langs
#     executable(name, 'interface/main.cpp',
#         dependencies : [ editor, generic, lib ]
#     )
# endforeach
