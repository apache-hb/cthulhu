#include "base/typeinfo.h"
#include "base/util.h"

static size_t info_ptr_hash(const void *key) { return ptrhash(key); }
static bool info_ptr_equal(const void *lhs, const void *rhs) { return lhs == rhs; }

static size_t info_str_hash(const void *key) { return str_hash(key); }
static bool info_str_equal(const void *lhs, const void *rhs) { return str_equal(lhs, rhs); }

const typeinfo_t kTypeInfoString = {
    .size = sizeof(char*),
    .hash = info_str_hash,
    .equals = info_str_equal,
};

const typeinfo_t kTypeInfoPtr = {
    .size = sizeof(void*),
    .hash = info_ptr_hash,
    .equals = info_ptr_equal,
};

