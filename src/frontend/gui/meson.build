gui_args = cpp.get_supported_arguments([
    '/wd4611', '/wd4201', '/wd4702', '-D_HAS_EXCEPTIONS=0',
    '-Wno-deprecated-enum-enum-conversion', '-Wno-unused-function',
    '-Wno-assume', '-D_ITERATOR_DEBUG_LEVEL=0'
])

deps = [ dependency('imgui'), dependency('implot') ]

libimgui = static_library('imgui',
    cpp_args: [ '-DIMGUI_USER_CONFIG="ctuconfig.h"' ],
    include_directories: 'vendor',
    dependencies: deps,
    build_by_default: false,
)

imgui = declare_dependency(
    link_with: libimgui,
    dependencies: deps,
)

# we provide imgui here ourselves so we can configure the assert handler
subdir('vendor/imgui')

memedit = declare_dependency(
    include_directories : 'vendor/imgui_memory_editor'
)

# we also modify imfilebrowser to not need exceptions enabled
subdir('vendor/imfilebrowser')
subdir('vendor/implot')

# this is just the d3d12 imgui example, but with a few changes
# to allow us to use it as a library.
# TODO: i should add more of the samples to support platforms
# other than windows.
subdir('draw')

subdir('editor')
