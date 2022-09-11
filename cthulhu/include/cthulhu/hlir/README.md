# Compilation stages
* module parsing
    * parse source files and create frontend ast

* forwarding declarations
    * declarations are all declared as unresolved in their appropriate namespaces

* compiling declarations
    * declarations and lazily compiled from the top down

* module creation
    * each compilation unit creates a module and then hands off back to the interface
