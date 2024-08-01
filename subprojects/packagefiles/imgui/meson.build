project('imgui', 'cpp',
    license: 'MIT',
    version: '1.91.0',
    meson_version: '>=0.59.0',
)

if host_machine.system() == 'darwin'
    add_languages('objcpp')
endif

sources = files(
    'misc/cpp/imgui_stdlib.cpp',
    'imgui.cpp',
    'imgui_demo.cpp',
    'imgui_draw.cpp',
    'imgui_tables.cpp',
    'imgui_widgets.cpp',
)

cpp = meson.get_compiler('cpp')
dependencies = []

# renderer backends
dependencies += cpp.find_library('d3dcompiler', required: host_machine.system() == 'windows')

dx11_dep = cpp.find_library('d3d11', required: get_option('dx11'))
if dx11_dep.found()
    sources += 'backends/imgui_impl_dx11.cpp'
    dependencies += dx11_dep
endif

dx12_dep = cpp.find_library('d3d12', required: get_option('dx12'))
# MinGW does not work. See https://github.com/ocornut/imgui/pull/4604
if dx12_dep.found() and cpp.get_argument_syntax() == 'msvc'
    sources += 'backends/imgui_impl_dx12.cpp'
    dependencies += dx12_dep
endif

metal_dep = dependency('appleframeworks', modules: [ 'Foundation', 'AppKit', 'GameController', 'Metal' ], required: get_option('metal'))
if metal_dep.found()
    sources += 'backends/imgui_impl_metal.mm'
    dependencies += metal_dep
endif

libgl_dep = dependency('gl', required: get_option('opengl'))
if libgl_dep.found()
    sources += 'backends/imgui_impl_opengl3.cpp'
    dependencies += libgl_dep
    dependencies += cpp.find_library('dl', required: false)
endif

vulkan_dep = dependency('vulkan', required: get_option('vulkan'))
if vulkan_dep.found()
    sources += 'backends/imgui_impl_vulkan.cpp'
    dependencies += vulkan_dep
endif

# platform backends
glfw_dep = dependency('glfw3', required: get_option('glfw'))
if glfw_dep.found()
    sources += 'backends/imgui_impl_glfw.cpp'
    dependencies += glfw_dep
endif

osx_dep = dependency('appleframeworks', modules: [ 'Carbon', 'Cocoa', 'GameController' ], required: get_option('osx'))
if osx_dep.found()
    sources += 'backends/imgui_impl_osx.mm'
endif

win_dep = cpp.find_library('dwmapi', required: get_option('win'))
if win_dep.found()
    sources += 'backends/imgui_impl_win32.cpp'
    dependencies += win_dep
endif

imgui_dep = declare_dependency(
    sources: sources,
    include_directories: [ '.', 'backends' ],
    dependencies: dependencies,
)

meson.override_dependency('imgui', imgui_dep)
