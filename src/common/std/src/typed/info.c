#include "std/typed/info.h"
#include "base/util.h"
#include "std/str.h"

static size_t info_ptr_hash(const void *key) { return ptrhash(key); }
static bool info_ptr_equal(const void *lhs, const void *rhs) { return lhs == rhs; }

static size_t info_str_hash(const void *key) { return strhash(key); }
static bool info_str_equal(const void *lhs, const void *rhs) { return str_equal(lhs, rhs); }

const type_info_t kTypeInfoPtr = {
    .hash = info_ptr_hash,
    .equals = info_ptr_equal,
};

const type_info_t kTypeInfoString = {
    .hash = info_str_hash,
    .equals = info_str_equal,
};
