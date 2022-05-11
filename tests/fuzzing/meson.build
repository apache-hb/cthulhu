fuzzers = {}

foreach name, lib : langs
    interface = executable(name, 'interface/main.c',
        dependencies : [ cmd, lib ]
    )

    fuzzers += { name: interface }
endforeach
