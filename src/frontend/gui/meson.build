gui_args = user_args + cpp.get_supported_arguments([
    '/wd4611', '/wd4201', '-D_HAS_EXCEPTIONS=0',
    '-Wno-deprecated-enum-enum-conversion', '-Wno-unused-function'
])

# we provide imgui here ourselves so we can configure the assert handler
subdir('imgui')

# we also modify imfilebrowser to not need exceptions enabled
subdir('imfilebrowser')
subdir('implot')

# this is just the d3d12 imgui example, but with a few changes
# to allow us to use it as a library.
# TODO: i should add more of the samples to support platforms
# other than windows.
subdir('draw')

subdir('editor')
