#pragma once
#include <stdbool.h>
#include <stddef.h>
struct System;
struct Arena;
struct System {
const char *field0;
};
struct Arena {
const char *field0;
void *(*field1)(struct Arena*, size_t);
void(*field2)(struct Arena*, void *);
};
struct Arena _ZN6system12defaultArenaEPKc(const char *name);
const char *_ZN6system13getSystemNameE6System(struct System system);
struct System _ZN6system9newSystemEP5Arena(struct Arena *mem);
