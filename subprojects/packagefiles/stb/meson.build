project('stb', 'c',
    license : 'MIT OR UNLICENSE',
    license_files : [ 'LICENSE' ],
    default_options : [
        'warning_level=0'
    ]
)

fs = import('fs')

# stb_image

lib_image = static_library('stb_image', fs.copyfile('stb_image.h', 'stb_image.c'),
    c_args : [ '-DSTB_IMAGE_IMPLEMENTATION' ],
    build_by_default : false
)

stb_image = declare_dependency(
    include_directories : '.',
    link_with : lib_image
)

meson.override_dependency('stb_image', stb_image)

# stb_sprintf

lib_sprintf = static_library('stb_sprintf', fs.copyfile('stb_sprintf.h', 'stb_sprintf.c'),
    c_args : [ '-DSTB_SPRINTF_IMPLEMENTATION' ],
    build_by_default : false
)

stb_sprintf = declare_dependency(
    include_directories : '.',
    link_with : lib_sprintf
)

meson.override_dependency('stb_sprintf', stb_sprintf)

# stb_perlin

lib_perlin = static_library('stb_perlin', fs.copyfile('stb_perlin.h', 'stb_perlin.c'),
    c_args : [ '-DSTB_PERLIN_IMPLEMENTATION' ],
    build_by_default : false
)

stb_perlin = declare_dependency(
    include_directories : '.',
    link_with : lib_perlin
)

meson.override_dependency('stb_perlin', stb_perlin)

# stb_rect_pack

lib_rectpack = static_library('stb_rect_pack', fs.copyfile('stb_rect_pack.h', 'stb_rect_pack.c'),
    c_args : [ '-DSTB_RECT_PACK_IMPLEMENTATION' ],
    build_by_default : false
)

stb_rectpack = declare_dependency(
    include_directories : '.',
    link_with : lib_rectpack
)

meson.override_dependency('stb_rect_pack', stb_rectpack)

# stb_image_resize

lib_image_resize = static_library('stb_image_resize', fs.copyfile('stb_image_resize2.h', 'stb_image_resize2.c'),
    c_args : [ '-DSTB_IMAGE_RESIZE_IMPLEMENTATION' ],
    build_by_default : false
)

stb_image_resize = declare_dependency(
    include_directories : '.',
    link_with : lib_image_resize
)

meson.override_dependency('stb_image_resize', stb_image_resize)

# image_write

lib_image_write = static_library('stb_image_write', fs.copyfile('stb_image_write.h', 'stb_image_write.c'),
    c_args : [ '-DSTB_IMAGE_WRITE_IMPLEMENTATION' ],
    build_by_default : false
)

stb_image_write = declare_dependency(
    include_directories : '.',
    link_with : lib_image_write
)

meson.override_dependency('stb_image_write', stb_image_write)

# truetype

lib_truetype = static_library('stb_truetype', fs.copyfile('stb_truetype.h', 'stb_truetype.c'),
    c_args : [ '-DSTB_TRUETYPE_IMPLEMENTATION' ],
    build_by_default : false
)

stb_truetype = declare_dependency(
    include_directories : '.',
    link_with : lib_truetype
)

meson.override_dependency('stb_truetype', stb_truetype)
