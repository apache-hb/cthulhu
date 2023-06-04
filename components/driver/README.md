# Language driver layout

all drivers must conform to the same dir structure

* id/
    * src/
    * include/
    * data/
    * meson.build

## build file requirements

to be detected by interfaces each build file must add itself to `langs`

```py
src = [ 'src/main.c' ]

ex = library('example', src,
    dependencies : [ generic, mediate, interop ],
    c_args : args,
    include_directories : [ 'src', versiondir ]
)

langs += { 
    'example': {
        'dep': declare_dependency(link_with : ex), # dependency object
        'mod': 'kExampleModule' # module definition exported from the object
    }
}
```
