type GDTEntry := [[packed]] {
    limit1: word,
    base1: word,
    base2: byte,
    access: byte,
    limit2: byte:4,
    flags: byte:4,
    base3: byte
}

type GDTPtr := [[packed]] {
    limit: word,
    base: dword,
    offset: word
}

type GDT := [[packed]] {
    code: GDTEntry,
    data: GDTEntry
}

type DiskAddressPacket := [[packed]] {
    size: byte,
    zero: byte,
    sectors: word,
    addr: dword,
    start: qword
}


[[bits(16)]] [[vaddr(0x7C00)]] {
    def start16 {
        [[x86::preserve(dl)]]
        x86::intrin::cld()
        x86::intrin::cli()
        x86::intrin::clc()

        x86::intrin::int(0x13, ah := 0x41, bx := 0x55AA)

        x86::intrin::jc(fail_ext)

        if(x86::intrin::bx() != 0xAA55)
            fail_ext()

        x86::intrin::int(0x13, ah := 0x42, si := &packet)

        x86::intrin::jc(fail_disk)
    }

    def fail_ext {
        let msg := "disk extensions not supported"
        panic(msg)
    }

    def fail_disk {
        let msg := "failed to read from disk"
        panic(msg)
    }

    def panic(msg: str) {
        for c : msg {
            x86::intrin::int(0x10, ah := 0x0E, al := c)
        }

        while true {
            x86::intrin::cli()
            x86::intrin::hlt()
        }
    }

    let packet: DiskAddressPacket = { 
        size := cc::sizeof(DiskAddressPacket),
        zero := 0,
        sectors := sectors,
        addr := start32,
        start := 1
    }

    let gdt: GDT := {
        code: {

        },
        data: {

        }
    }

    [[bits]] {
        [511] := 0xAA
        [512] := 0x55
    }
}

[[bits(32)]] {
    def start32 {

    }
}

let sectors: u32 = [[paddr]] / 512