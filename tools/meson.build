executable('ast', 'legacy/ast.cpp',
    cpp_args : args, 
    dependencies : ctu,
    include_directories : '..'
)

executable('ctc', 'legacy/ctc.cpp',
    cpp_args : args,
    dependencies : ctu,
    include_directories : '..'
)

executable('tokenize', 'tokenize.cpp',
    cpp_args : args,
    dependencies : ctu2,
    include_directories : '..'
)

# the gui uses dx11 so windows only
if target_machine.system() == 'windows'
    imgui = subproject('imgui').get_variable('imgui_dep')

    executable('tokenize-gui', 'tokenize-gui.cpp',
        cpp_args : args,
        dependencies : [ ctu2, imgui ],
        include_directories : '..',
        link_args : 'd3d11.lib'
    )
endif
