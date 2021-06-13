if not msvc
    amd64 = subproject('qbe').get_variable('amd64')

    qbe = library('qbe-backend', 'qbe.c',
        dependencies : amd64
    )

    backends += declare_dependency(
        link_with : qbe,
        dependencies : amd64
    )
else
    backends += declare_dependency(
        link_with : library('qbe-fallback', 'fallback.c')
    )
endif
