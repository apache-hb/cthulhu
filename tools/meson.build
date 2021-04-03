executable('ast', 'legacy/ast.cpp',
    dependencies : ctu,
    include_directories : '..'
)

#executable('ctc', 'legacy/ctc.cpp',
#    dependencies : ctu,
#    include_directories : '..'
#)

executable('tokenize', 'legacy2/tokenize.cpp',
    dependencies : ctu2,
    include_directories : '..'
)

# the gui uses dx11 so windows only
if target_machine.system() == 'windows'
    imgui = subproject('imgui').get_variable('imgui_dep')

    executable('tokenize-gui', 'legacy2/tokenize-gui.cpp',
        dependencies : [ ctu2, imgui ],
        include_directories : '..',
        link_args : 'd3d11.lib'
    )

    executable('parse-gui', 'legacy2/parse-gui.cpp',
        dependencies : [ ctu2, imgui ],
        include_directories : '..',
        link_args : 'd3d11.lib'
    )

    executable('tokens-gui', 'tokens-gui.cpp',
        dependencies : [ ctu3, imgui ],
        link_args : 'd3d11.lib'
    )

    executable('tree-gui', 'tree-gui.cpp',
        dependencies : [ ctu3, imgui ],
        link_args : 'd3d11.lib'
    )
endif

executable('ctc', 'ctc.cpp', dependencies : ctu4)
