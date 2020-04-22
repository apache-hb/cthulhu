import x:y:z->jeff
import x86:intrin
import ctu:intrin->cc
import lib

type a := @nullable int*

type s := { a: int, b: float }

type t := (int, int)

type u := union { x: int, y: (int, int) }

type any := void*

type i := int
type j := u32
type b := b32

type VkBool32 := u32

def VK_TRUE := VkBool32(1)
def VK_FALSE := VkBool32(0)


type VkResult := enum: i32 {
    Success := 0,
    NotReady := 1,
    Timeout := 2,
    EventSet := 3,
    EventReset := 4,
    Incomplete := 4,

    ErrorOutOfHostMemory := -1,
    ErrorOutOfDeviceMemory := -2,
    ErrorInitializationFailed := -3,
    ErrorDeviceList := -4,
    ErrorMemoryMapFailed := -5
}