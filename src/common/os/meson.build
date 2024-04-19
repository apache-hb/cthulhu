os_api = configure_file(
    input : api_header,
    output : 'ctu_os_api.h',
    configuration : { 'MOD': 'OS' }
)

os_include = include_directories('.', 'include')
os_impl_include = include_directories('src')

src = [ 'src/os_common.c' ]

if target == 'windows'
    os_platform = 'windows'
elif target == 'darwin' or target == 'linux'
    os_platform = 'posix'
else
    error('unsupported target os ' + target)
endif

os_api_build = '-DCT_OS_BUILD=1'

# we use a switch case over bitflags in get_protect
# this is safe, but compilers warn about it
os_args = cc.get_supported_arguments('-Wno-switch', '/wd4063')

lib_oscommon = library('os_common', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + os_args + [ os_api_build ],
    dependencies : [ base ],
    include_directories : [ os_include ]
)

os_common = declare_dependency(
    link_with : lib_oscommon,
    dependencies : [ core ],
    include_directories : [ os_include ]
)

# get the backend data

subdir('src' / os_platform)

os_cdata = configuration_data()

# is there an optimized way to copy a file?
os_cdata.set10('CTU_OS_HAS_COPYFILE', os_config['copyfile'])

# is dynamic loading supported?
os_cdata.set10('CTU_OS_HAS_DYNLOAD', os_config['dynamic'])
os_config = configure_file(
    output : 'os_config.h',
    configuration : os_cdata
)

osnative_inc = include_directories('.')

lib_osnative = static_library('os_' + os_platform,
    build_by_default : false,
    c_args : user_args + os_args + [ os_api_build ],
    dependencies : [ base, std, os_common, os_native ],
    include_directories : [ os_include, os_impl_include, osnative_inc ]
)

# create the os interface library

src = [ 'src/os.c' ]

libos = library('os', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ os_api_build ] + os_args,
    dependencies : [ base, arena, std, os_common ],
    link_with : [ lib_osnative ]
)

os = declare_dependency(
    link_with : libos,
    dependencies : [ os_common ],
    include_directories : os_include
)

install_headers(os_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
