project('stb', 'c',
    license : 'MIT OR UNLICENSE',
    license_files : [ 'LICENSE' ],
    default_options : [
        'warning_level=0'
    ]
)

all = [
    'hexwave',
    'image',
    'truetype',
    'image_write',
    'image_resize2',
    'rect_pack',
    'perlin',
    'ds',
    'sprintf',
    'textedit',
    'voxel_render',
    'dxt',
    'easy_font',
    'tilemap_editor',
    'herringbone_wang_tile',
    'c_lexer',
    'divide',
    'connected_components',
    'leakcheck',
    'include'
]

fs = import('fs')

foreach target : all
    name = 'stb_' + target
    file = fs.copyfile(name + '.h', name + '.c')

    lib = static_library(name, file,
        c_args : [ '-D' + name.to_upper() + '_IMPLEMENTATION' ],
        build_by_default : false
    )

    dep = declare_dependency(
        include_directories : '.',
        link_with : lib
    )

    meson.override_dependency(name, dep)
endforeach
