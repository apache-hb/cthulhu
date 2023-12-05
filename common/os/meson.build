
src = [ 'src/os_common.c' ]

lib_oscommon = library('os_common', src,
    c_args : user_args,
    dependencies : [ core, base, memory, std ],
    include_directories : [ 'include', 'src' ]
)

os_common = declare_dependency(
    link_with : lib_oscommon,
    dependencies : [ core ],
    include_directories : [ 'include', 'src' ]
)

# get the backend

subdir('src' / target)

# create the native library

os = declare_dependency(
    link_with : lib_osnative,
    include_directories : 'include',
    dependencies : [ core ]
)
