imgui = subproject('imgui')
core = imgui.get_variable('imgui')
impl = imgui.get_variable('backend')

glfw = dependency('glfw3', fallback : [ 'glfw', 'glfw3' ])
glad = subproject('glad').get_variable('glad')

foreach name, lib : langs
    executable(name, 'interface/main.cpp',
        link_with : wrapper,
        dependencies : [ generic, lib, core, impl, glfw, glad ]
    )
endforeach
