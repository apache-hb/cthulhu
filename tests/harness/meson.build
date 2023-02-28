links = host != 'windows' ? [] : [ 'setargv.obj' ]
harness = {}

foreach name, lib : langs
    it = executable(name, 'harness.c',
        export_dynamic : true,
        link_args : links,
        dependencies : [ generic, lib, ssa, emit ]
    )
    harness += { name : it }
endforeach
