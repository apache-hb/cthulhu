deps = [ base, stb_sprintf ]
src = [
    'imgui/misc/cpp/imgui_stdlib.cpp',
    'imgui/imgui.cpp',
    'imgui/imgui_demo.cpp',
    'imgui/imgui_draw.cpp',
    'imgui/imgui_tables.cpp',
    'imgui/imgui_widgets.cpp',
]

deps += cpp.find_library('d3dcompiler', required: host_machine.system() == 'windows')
deps += cpp.find_library('dxgi', required: host_machine.system() == 'windows')

dx11_dep = cpp.find_library('d3d11', required: host_machine.system() == 'windows')
if dx11_dep.found()
    src += 'imgui/backends/imgui_impl_dx11.cpp'
    deps += dx11_dep
endif

dx12_dep = cpp.find_library('d3d12', required: host_machine.system() == 'windows')
if dx12_dep.found() and cpp.get_argument_syntax() == 'msvc'
    src += 'imgui/backends/imgui_impl_dx12.cpp'
    deps += dx12_dep
endif

# metal_dep = dependency('appleframeworks', modules: [ 'Foundation', 'AppKit', 'GameController', 'Metal' ], required: get_option('metal'))
# if metal_dep.found()
#     sources += 'backends/imgui_impl_metal.mm'
#     dependencies += metal_dep
# endif

# libgl_dep = dependency('gl', required: get_option('opengl'))
# if libgl_dep.found()
#     sources += 'backends/imgui_impl_opengl3.cpp'
#     dependencies += libgl_dep
#     dependencies += cpp.find_library('dl', required: false)
# endif

# vulkan_dep = dependency('vulkan', required: get_option('vulkan'))
# if vulkan_dep.found()
#     sources += 'backends/imgui_impl_vulkan.cpp'
#     dependencies += vulkan_dep
# endif

# glfw_dep = dependency('glfw3', required: get_option('glfw'))
# if glfw_dep.found()
#     sources += 'backends/imgui_impl_glfw.cpp'
#     dependencies += glfw_dep
# endif

# osx_dep = dependency('appleframeworks', modules: [ 'Carbon', 'Cocoa', 'GameController' ], required: get_option('osx'))
# if osx_dep.found()
#     sources += 'backends/imgui_impl_osx.mm'
# endif

win_dep = cpp.find_library('dwmapi', required: host_machine.system() == 'windows')
if win_dep.found()
    src += 'imgui/backends/imgui_impl_win32.cpp'
    deps += win_dep
endif

inc = include_directories('.', 'imgui', 'imgui/backends')

args = [ '-DIMGUI_USER_CONFIG="ctuconfig.h"' ]

libimgui = static_library('imgui', src,
    cpp_args: args,
    include_directories: inc,
    dependencies: deps,
)

imgui = declare_dependency(
    dependencies: deps,
    compile_args: args,
    include_directories: inc,
    link_with: libimgui,
)

# implot

src = [
    'implot/implot.cpp',
    'implot/implot_items.cpp',
    'implot/implot_demo.cpp',
]

inc = include_directories('implot')

libimplot = static_library('implot', src,
    include_directories: inc,
    dependencies: imgui,
)

implot = declare_dependency(
    include_directories: inc,
    link_with: libimplot,
    dependencies: imgui
)