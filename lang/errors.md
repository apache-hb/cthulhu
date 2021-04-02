# Error handling

Error handling is managed using the `error` polymorphic variant result type.

## Ok path handling

valid values are returned using `return expr`, empty options are returned with `return`.
A function may not `return` an expression that does not evaluate to its return type.

## Error path handling

Error values are returned using `raise expr`.
A function may not `raise` an error that is not included in its error variant.

## Error propogation

errors can be propogated using `try expr` where if `expr` evaluates to its `ok` type the expression evaluates to it. otherwise the expression returns the error value to the caller of the current function.

Error values may be unwrapped with `expr ?: other` which evaluates to `expr` if its `ok` type is returned. otherwise `other` is evaluated and the expression evaluates to the result of `other`. `other` may not be evaluated with `expr` does not evaluate to an error.
