#pragma once

#define CTX_UNUSED [[maybe_unused]]

#define CTX_NOMOVE(x) \
    x(x&&) = delete; \
    x& operator=(x&&) = delete;

#define CTX_NOCOPY(x) \
    x(const x&) = delete; \
    x& operator=(const x&) = delete;
