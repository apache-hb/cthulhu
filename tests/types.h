import x:y:z -> jeff

type x := {
    a: int,
    b: float
}

type y := @packed(1) {
    addr: u64,
    len: u64,
    type: enum: u32 {
        ram := 1,
        reserved := 2,
        acpi := 3,
        nvs := 4,
        bad := 5
    },
    attrib: u32
}