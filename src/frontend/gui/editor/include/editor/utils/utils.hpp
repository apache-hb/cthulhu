#pragma once

#define CTU_NOMOVE(x) \
    x(x&&) = delete; \
    x& operator=(x&&) = delete;

#define CTU_NOCOPY(x) \
    x(const x&) = delete; \
    x& operator=(const x&) = delete;
