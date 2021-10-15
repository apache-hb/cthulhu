#pragma once

#define ALIGN(addr, bound) ((addr + bound - 1) / bound) * bound
#define ALIGN4K(addr) ((addr + 0x1000 - 1) & -0x1000)

#define MMAP_ARENA(size) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
#define GROW_ARENA(ptr, previous, size) mremap(ptr, previous, size, 0)
#define UNMAP_ARENA(ptr, size) munmap(ptr, size)
