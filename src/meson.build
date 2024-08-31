# pull stb_sprintf in here if its required, common and frontend use it
stb_sprintf = dependency('stb_sprintf', allow_fallback : true, required : opt_stb_sprintf)

# base libraries
subdir('common')

# grap gmp here to specifically avoid it being used in common
gmp = dependency('gmp', fallback : [ 'mini_gmp', 'gmp' ])

# compiler libraries
subdir('cthulhu')

# support libraries
subdir('support')

# codegen drivers
subdir('target')

# language drivers
subdir('language')

# compiler plugins
subdir('plugins')

# module loader
subdir('loader')

# frontend programs
subdir('frontend')

# user and compiler tools
subdir('tools')
