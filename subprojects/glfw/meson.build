project('glfw', 'c')

host = host_machine.system()
cc = meson.get_compiler('c')

src = [
    'glfw/src/init.c',
    'glfw/src/input.c',
    'glfw/src/monitor.c',
    'glfw/src/context.c',
    'glfw/src/platform.c',
    'glfw/src/vulkan.c',
    'glfw/src/window.c',
    'glfw/src/egl_context.c',
    'glfw/src/osmesa_context.c',
    'glfw/src/null_init.c',
    'glfw/src/null_monitor.c',
    'glfw/src/null_window.c',
    'glfw/src/null_joystick.c'
]

links = []
libs = []
args = []

if host == 'windows'
    links += [ 'gdi32' ]
    args += [ '-D_CRT_SECURE_NO_WARNINGS=1', '-D_GLFW_WIN32=1' ]
    src += [
        'glfw/src/win32_module.c',
        'glfw/src/win32_time.c',
        'glfw/src/win32_thread.c',

        'glfw/src/win32_init.c',
        'glfw/src/win32_joystick.c',
        'glfw/src/win32_monitor.c',
        'glfw/src/win32_window.c',
        'glfw/src/wgl_context.c'
    ]
elif host == 'linux'
    libs += [ cc.find_library('rt'), cc.find_library('m'), cc.find_library('dl') ]
    args += [ '-D_GLFW_X11=1' ]
    src += [
        'glfw/src/posix_module.c',
        'glfw/src/posix_time.c',
        'glfw/src/posix_thread.c',

        'glfw/src/x11_init.c',
        'glfw/src/x11_monitor.c',
        'glfw/src/x11_window.c',
        'glfw/src/xkb_unicode.c',
        'glfw/src/glx_context.c'
    ]
endif

libglfw = library('glfw', src,
    c_args : args,
    link_args : links,
    dependencies : libs
)

glfw3 = declare_dependency(
    link_with : libglfw,
    include_directories : [ 'glfw/include' ]
)
