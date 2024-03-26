os_config = {
    'name': 'posix',
    'copyfile': false,
    'dynamic': true,
}

src = files('init.c', 'file.c', 'dir.c', 'fs.c', 'error.c', 'library.c')

os_native = declare_dependency(
    sources : src,
    dependencies : [ cc.find_library('dl') ]
)
