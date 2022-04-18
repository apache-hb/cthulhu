# TODO items

## Blocking items

* complete wat support
  * how do strings work

* document more stuff
  * other people have to contribute now

* extract most of `generic` into subprojects
  * better seperation of code into more logical blocks
  * makes it easier to generate variations of the compiler
  * required for plugins

* create plugin system
  * this would allow our secret sauce to be kept closed source
  * while the majority of the framework would be open source
    * other people can fix our bugs for us
    * brownie points for being open source
    * means we can use libraries like libgccjit
    * my work will remain available to me even after the summer ends
  * this would also allow for codegen shims to be added in
    * makes gathering metrics possible and exensible

* get this all running inside aws lambda
  * i dont know how s3 buckets work
  * i am too scared of spending my life savings in 5 seconds

## Optional items

* add ssa emitter
  * direct asm/wasm output would be preferrable to relying on C/wat2wasm

* optimisation passes
  * more speed never hurts
  * would get us on the compiler framework map so to speak
  * would save us money in the long run with lambda runtimes

* rework command line parsing
  * instead of manually scanning the command line we should use flex & bison
    * a touch overkill perhaps
    * much easier to add/remove flags
    * would allow drivers to add their own flags
  * also allow passing other formats as flags
    * xml
    * json
    * yaml (ideally not this)
