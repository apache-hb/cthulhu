[[extern("C", name = "SECTORS")]]
var sectors: u32

[[bits(16)]] [[section(".real")]] {
    def start16 {
        x86::intrin::cld()
        x86::intrin::cli()
        x86::intrin::clc()

        x86::intrin::int(0x13, ah := 0x41, bx := 0x55AA)

        x86::intrin::jc(fail_ext)

        if(x86::intrin::bx() != 0xAA55)
            fail_ext()

        packet.sectors := sectors

        x86::intrin::int(0x13, ah := 0x42, si := packet)

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
        for char : msg {
            x86::intrin::int(0x10, ah := 0x0E, al := char)
        }

        while true {
            x86::intrin::cli()
            x86::intrin::hlt()
        }
    }

    [[packed]]
    type DiskAddressPacket := {
        size: u8,
        zero: u8,
        sectors: u16,
        addr: u32,
        start: u64
    }

    var packet: DiskAddressPacket = { 
        size := [[size(DiskAddressPacket)]],
        zero := 0,
        addr := start32,
        start := 1
    }

    [[asm]] {
        [511] := 0xAA
        [512] := 0x55
    }
}

[[bits(32)]] [[section(".prot")]] {
    def start32 {

    }
}