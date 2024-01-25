# base libraries
subdir('common')

# grap gmp here to specifically avoid it being used in common
gmp = dependency('gmp', fallback : [ 'mini_gmp', 'gmp' ])

# compiler libraries
subdir('cthulhu')

# codegen drivers
subdir('target')

# language drivers
subdir('language')

# frontend and tool support libraries
subdir('support')

# frontend programs
subdir('frontend')

# user and compiler tools
subdir('tools')
