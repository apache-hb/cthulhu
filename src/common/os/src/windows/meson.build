os_config = {
    'name': 'windows',
    'copyfile': true,
    'dynamic': true,
    'threads': true,
}

src = files('init.c', 'file.c', 'dir.c', 'fs.c', 'error.c', 'library.c', 'thread.c')

os_native = declare_dependency(
    sources : src,
    dependencies : [ core ]
)
