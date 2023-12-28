
src = [ 'src/os_common.c' ]

if target == 'windows'
    os_platform = 'windows'
elif target == 'darwin' or target == 'linux'
    os_platform = 'posix'
else
    error('unknown os api')
endif

api_header = 'impl' / os_platform + '.h'
os_header = '-DOS_API_HEADER="' + api_header + '"'

lib_oscommon = library('os_common', src,
    c_args : user_args + [ os_header ],
    dependencies : [ core, base, memory, std ],
    include_directories : [ 'include', 'src' ]
)

os_common = declare_dependency(
    link_with : lib_oscommon,
    compile_args : [ os_header ],
    dependencies : [ core ],
    include_directories : [ 'include', 'src' ]
)

# get the backend

subdir('src' / os_platform)

# create the native library

os = declare_dependency(
    compile_args : [ os_header ],
    link_with : [ lib_osnative, lib_oscommon ],
    include_directories : 'include',
    dependencies : [ core ]
)
