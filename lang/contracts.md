# Contracts

## Function refinements
```py
def parse(text: str, radix: uint): ulong 
refine
    assert: text != null;
    assert: radix > 0 && radix <= 16;
{

}
```

## Type refinements
```py
record Date 
refine
    assert: year > 0;
    assert: month >= 0 && month <= 12;
    assert: day >= 0 && month <= 31;
{
    year: int;
    month: int;
    day: int;
}
```