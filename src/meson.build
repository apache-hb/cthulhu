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
