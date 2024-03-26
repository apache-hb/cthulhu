os_config = {
    'name': 'windows',
    'copyfile': true,
    'dynamic': true,
}

src = files('init.c', 'file.c', 'dir.c', 'fs.c', 'error.c', 'library.c')

os_native = declare_dependency(
    sources : src,
    dependencies : [ arena ]
)
