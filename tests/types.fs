import x:y:z -> jeff
import x86:intrin
import ctu:intrin -> cc
import lib

type x := {
    a: int,
    b: float
}

type e := enum: u64 {
    a := 5,
    b := 6,
    c := 7
}

def rotate_e(v: e) := match v {
    e:a -> e:b
    e:b -> e:c
    e:c -> e:a
}

type E820Entry := @packed(1) {
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