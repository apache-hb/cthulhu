// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "std/typeinfo.h"
#include "base/util.h"

size_t info_ptr_hash(const void *key) { return ctu_ptrhash(key); }
bool info_ptr_equal(const void *lhs, const void *rhs) { return lhs == rhs; }

size_t info_str_hash(const void *key) { return str_hash(key); }

bool info_str_equal(const void *lhs, const void *rhs)
{
    if (lhs == rhs) return true;
    if (lhs == NULL || rhs == NULL) return false;

    return str_equal(lhs, rhs);
}

size_t info_text_hash(const void *key)
{
    text_view_t *view = (text_view_t*)key;
    return text_hash(*view);
}

bool info_text_equal(const void *lhs, const void *rhs)
{
    text_view_t *l = (text_view_t*)lhs;
    text_view_t *r = (text_view_t*)rhs;
    return text_equal(*l, *r);
}

const hash_info_t kTypeInfoString = {
    .size = sizeof(char*),
    .hash = info_str_hash,
    .equals = info_str_equal,
};

const hash_info_t kTypeInfoPtr = {
    .size = sizeof(void*),
    .hash = info_ptr_hash,
    .equals = info_ptr_equal,
};

const hash_info_t kTypeInfoText = {
    .size = sizeof(text_view_t),
    .hash = info_text_hash,
    .equals = info_text_equal,
};
