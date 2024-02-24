os_api = configure_file(
    input : api_header,
    output : 'ctu_os_api.h',
    configuration : { 'MOD': 'OS' }
)

os_include = include_directories('.', 'include')
os_impl_include = include_directories('src')

src = [
    'src/os_common.c'
]

if target == 'windows'
    os_platform = 'windows'
elif target == 'darwin' or target == 'linux'
    os_platform = 'posix'
else
    error('unknown os api')
endif

os_api_build = '-DCT_OS_BUILD=1'

# TODO: the impl libraries should be static, and os_common should route
# to the native impl, allowing common checks to be done in common
# would do now but time crunch
lib_oscommon = library('os_common', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ os_api_build ],
    dependencies : [ core, base, arena, std ],
    include_directories : [ os_include, os_impl_include ]
)

os_common = declare_dependency(
    link_with : lib_oscommon,
    dependencies : [ core ],
    include_directories : [ os_include, os_impl_include ]
)

# get the backend

subdir('src' / os_platform)

# create the native library

os = declare_dependency(
    link_with : [ lib_osnative ],
    dependencies : [ os_common ],
    include_directories : os_include
)

install_headers(os_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
