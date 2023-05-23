src = [ 
    'src/main.c'
]

ex = shared_library('example', src,
    dependencies : [ generic, mediate, interop ],
    c_args : args,
    include_directories : [ 'src', versiondir ]
)

langs += { 'example': ex }
