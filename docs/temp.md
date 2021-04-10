```cs
using std::prelude(u64 => uint64_t);

var i = 10;
var a1 = i as long;
var a2 = i <- long;
var a3 = i : long;
var a4 = i => long;
```

```cs

record Name {
    field: int
}

operator(==)
def equals(self: &Name, other: &Name): bool 
    = self.field == other.field;
```
