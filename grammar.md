## type grammar

struct-body = ident `:` type [`,` struct-body]

struct = `{` [struct-body] `}`


tuple-body = type [`,` tuple-body]

tuple = `(` [tuple-body] `)`


union = `union` struct


variant-body = ident `=>` type [`,` variant-body]

variant = `variant` [`:` type] `{` [variant-body] `}`


enum-body = ident `:=` expr [`,` enum-body]

enum = `enum` [`:` type] `{` [enum-body] `}`


ptr = `*` type


array = `[` type `:` expr `]`


typename = ident

type = struct | tuple | union | variant | enum | ptr | array | typename

typedef = `type` `=` type