#include "std/str.h"
#include "std/map.h"
#include "std/typed/vector.h"
#include "std/vector.h"

#include "core/macros.h"
#include "memory/memory.h"
#include "base/panic.h"

#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

USE_DECL
char *str_format(arena_t *arena, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char *str = str_vformat(arena, fmt, args);

    va_end(args);

    return str;
}

USE_DECL
char *str_vformat(arena_t *arena, const char *fmt, va_list args)
{
    CTASSERT(arena != NULL);
    CTASSERT(fmt != NULL);

    // make a copy of the args for the second format
    va_list again;
    va_copy(again, args);

    // get the number of bytes needed to format
    int len = vsnprintf(NULL, 0, fmt, args) + 1;

    CTASSERTF(len > 0, "vformat failed to format string: %s", fmt);

    char *out = arena_malloc(len, "vformat", arena, arena);

    int result = vsnprintf(out, len, fmt, again);
    CTASSERTF(result == len - 1, "vformat failed to format string: %s (%d == %d - 1)", fmt, result, len);

    va_end(again);

    return out;
}

USE_DECL
char *format(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *str = vformat(fmt, args);
    va_end(args);

    return str;
}

USE_DECL
char *vformat(const char *fmt, va_list args)
{
    arena_t *arena = get_global_arena();
    return str_vformat(arena, fmt, args);
}

bool char_is_any_of(char c, const char *chars)
{
    CTASSERT(chars != NULL);

    for (; *chars; chars++)
    {
        if (*chars == c)
        {
            return true;
        }
    }

    return false;
}

static size_t str_rfind_any(const char *str, const char *letters)
{
    CTASSERT(str != NULL);
    CTASSERT(letters != NULL);

    size_t i = strlen(str);
    while (i > 0)
    {
        i--;
        if (char_is_any_of(str[i], letters))
        {
            return i;
        }
    }

    return SIZE_MAX;
}

USE_DECL
char *str_noext(const char *path)
{
    CTASSERT(path != NULL);

    size_t idx = str_rfind(path, ".");
    char *base = ctu_strdup(path);
    if (idx == SIZE_MAX)
    {
        return base;
    }

    base[idx] = '\0';
    return base;
}

USE_DECL
char *str_ext(const char *path)
{
    CTASSERT(path != NULL);

    size_t idx = str_rfind(path, ".");
    if (idx == SIZE_MAX)
    {
        return NULL;
    }

    return ctu_strdup(path + idx + 1);
}

USE_DECL
char *str_filename_noext(const char *path)
{
    CTASSERT(path != NULL);

    size_t idx = str_rfind_any(path, PATH_SEPERATORS);
    if (idx == SIZE_MAX)
    {
        return str_noext(path);
    }

    return str_noext(path + idx + 1);
}

USE_DECL
char *str_filename(const char *path)
{
    CTASSERT(path != NULL);

    size_t idx = str_rfind_any(path, PATH_SEPERATORS);
    if (idx == SIZE_MAX)
    {
        return ctu_strdup(path);
    }

    return ctu_strdup(path + idx + 1);
}

USE_DECL
bool str_startswith(const char *str, const char *prefix)
{
    CTASSERT(str != NULL);
    CTASSERT(prefix != NULL);

    return strncmp(str, prefix, strlen(prefix)) == 0;
}

USE_DECL
bool str_endswith(const char *str, const char *suffix)
{
    CTASSERT(str != NULL);
    CTASSERT(suffix != NULL);

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
    {
        return false;
    }

    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

USE_DECL
char *str_join(const char *sep, vector_t *parts)
{
    CTASSERT(sep != NULL);
    CTASSERT(parts != NULL);

    size_t all = vector_len(parts);

    if (all == 0)
    {
        return ctu_strdup("");
    }
    else if (all == 1)
    {
        char *it = vector_get(parts, 0);
        CTASSERT(it != NULL);
        return ctu_strdup(it);
    }

    size_t len = 0;
    size_t seplen = strlen(sep);
    for (size_t i = 0; i < all; i++)
    {
        const char *part = vector_get(parts, i);
        CTASSERTF(part != NULL, "part[%zu] = NULL", i);

        len += strlen(part);

        if (i != 0)
        {
            len += seplen;
        }
    }

    CTASSERTF(len > 0, "len = %zu", len);

    char *out = ARENA_MALLOC(get_global_arena(), len + 1, "str_join", NULL);
    size_t idx = 0;

    size_t remaining = len;
    for (size_t i = 0; i < all; i++)
    {
        if (i != 0)
        {
            memcpy(out + idx, sep, MIN(remaining, seplen));
            idx += seplen;
            remaining -= seplen;
        }

        const char *part = vector_get(parts, i);
        size_t part_len = strlen(part);
        memcpy(out + idx, part, MIN(remaining, part_len));
        idx += part_len;
        remaining -= part_len;
    }

    out[idx] = 0;
    return out;
}

USE_DECL
char *str_repeat(const char *str, size_t times)
{
    CTASSERT(str != NULL);

    if (times == 0)
    {
        return ctu_strdup("");
    }

    size_t len = strlen(str);
    size_t outlen = len * times;
    char *out = ARENA_MALLOC(get_global_arena(), outlen + 1, "str_repeat", NULL);
    size_t remaining = outlen;
    for (size_t i = 0; i < times; i++)
    {
        memcpy(out + i * len, str, MIN(remaining, len));
        remaining -= len;
    }
    out[outlen] = 0;
    return out;
}

static const char kEscapeChars[] = {
    '\n', '\t', '\f', '\r', '\v', '\\', '\0', '\'', '\"',
};

static bool is_escape_char(char c)
{
    for (size_t i = 0; i < sizeof(kEscapeChars); i++)
    {
        if (kEscapeChars[i] == c)
        {
            return true;
        }
    }

    return false;
}

static bool safe_isprint(char c)
{
    if (is_escape_char(c))
    {
        return false;
    }

    if (c > 0x1F && c != 0x7F)
    {
        return true;
    }

    return false;
}

static size_t normlen(char c)
{
    // TODO: this might overallocate size
    return safe_isprint(c) ? 1 : 4;
}

static size_t normstr(char *out, char c)
{
    if (safe_isprint(c))
    {
        *out = c;
        return 1;
    }

    switch (c)
    {
    case '\\':
        out[0] = '\\';
        out[1] = '\\';
        return 2;
    case '\'':
        out[0] = '\\';
        out[1] = '\'';
        return 2;
    case '\"':
        out[0] = '\\';
        out[1] = '\"';
        return 2;
    case '\0':
        out[0] = '\\';
        out[1] = '0';
        return 2;
    case '\n':
        out[0] = '\\';
        out[1] = 'n';
        return 2;
    case '\t':
        out[0] = '\\';
        out[1] = 't';
        return 2;
    case '\r':
        out[0] = '\\';
        out[1] = 'r';
        return 2;
    default: {
        int result = snprintf(out, 5, "\\x%02x", (c & 0xFF));
        CTASSERT(result > 0);
        return result;
    }
    }
}

USE_DECL
char *str_normalize(const char *input)
{
    CTASSERT(input != NULL);

    size_t input_length = 0;
    size_t result_length = 0;
    const char *length_iter = input;
    while (*length_iter != '\0')
    {
        size_t inc = normlen(*length_iter++);
        result_length += inc;
        input_length += 1;
    }

    // if the string is already normalized, just return a copy
    if (input_length == result_length)
    {
        return ctu_strndup(input, input_length);
    }

    const char *repl_iter = input;
    char *buf = ARENA_MALLOC(get_global_arena(), result_length + 1, "str_normalize", NULL);
    char *result = buf;
    while (*repl_iter != '\0')
    {
        result += normstr(result, *repl_iter++);
    }
    *result = '\0';

    return buf;
}

USE_DECL
char *str_normalizen(const char *str, size_t len)
{
    CTASSERT(str != NULL);

    size_t length = 1;
    for (size_t i = 0; i < len; i++)
    {
        length += normlen(str[i]);
    }

    // if the string is already normalized, just return a copy
    if (length == len)
    {
        return ctu_strndup(str, len);
    }

    char *buf = ARENA_MALLOC(get_global_arena(), length + 1, "str_normalizen", NULL);
    size_t offset = 0;
    for (size_t i = 0; i < len; i++)
    {
        size_t inc = normstr(buf + offset, str[i]);
        offset += inc;
    }

    buf[offset] = '\0';
    return buf;
}

USE_DECL
vector_t *str_split_arena(IN_STRING const char *str, IN_STRING const char *sep, arena_t *arena)
{
    CTASSERT(str != NULL);
    CTASSERT(sep != NULL);
    CTASSERT(arena != NULL);

    if (strlen(sep) == 0)
    {
        // split into individual characters
        vector_t *result = vector_new_arena(strlen(str), arena);
        for (size_t i = 0; i < strlen(str); i++)
        {
            char *c = ARENA_MALLOC(arena, 2, "str_split", NULL);
            c[0] = str[i];
            c[1] = '\0';
            vector_push(&result, c);
        }
        return result;
    }

    size_t seplen = strlen(sep);
    vector_t *result = vector_new_arena(4, arena);

    // store the start of the current token
    // continue until a seperator is found
    // then push that token to the vector
    // and start the next token

    const char *token = str;
    const char *cursor = str;

    while (*cursor)
    {
        if (!str_startswith(cursor, sep))
        {
            cursor += 1;
            continue;
        }

        vector_push(&result, arena_strndup(token, cursor - token, arena));
        token = cursor + seplen;
        cursor += seplen;
    }

    vector_push(&result, arena_strndup(token, cursor - token, arena));

    return result;
}

USE_DECL
vector_t *str_split(const char *str, const char *sep)
{
    arena_t *arena = get_global_arena();
    return str_split_arena(str, sep, arena);
}

USE_DECL
size_t strhash(const char *str)
{
    CTASSERT(str != NULL);

    size_t hash = 0;

    while (*str)
    {
        hash = (hash << 5) - hash + *str++;
    }

    return hash;
}

USE_DECL
bool str_contains(const char *str, const char *search)
{
    CTASSERT(str != NULL);
    CTASSERT(search != NULL);

    return strstr(str, search) != NULL;
}

USE_DECL
char *str_replace(const char *str, const char *search, const char *repl)
{
    CTASSERT(str != NULL);
    CTASSERT(search != NULL);
    CTASSERT(repl != NULL);

    if (strlen(search) == 0)
    {
        return ctu_strdup(str);
    }

    vector_t *split = str_split(str, search);
    return str_join(repl, split);
}

static const map_entry_t *find_matching_key(typevec_t *pairs, const char *str)
{
    for (size_t i = 0; i < typevec_len(pairs); i++)
    {
        const map_entry_t *entry = typevec_offset(pairs, i);
        CTASSERT(entry != NULL);
        CTASSERT(entry->key != NULL);
        CTASSERT(entry->value != NULL);

        if (str_startswith(str, entry->key))
        {
            return entry;
        }
    }

    return NULL;
}

USE_DECL
char *str_replace_many(const char *str, map_t *repl)
{
    CTASSERT(str != NULL);
    CTASSERT(repl != NULL);

    size_t len = 0;
    typevec_t *pairs = map_entries(repl);

    const char *iter = str;
    while (*iter)
    {
        const map_entry_t *entry = find_matching_key(pairs, iter);
        if (entry != NULL)
        {
            len += strlen(entry->value);
            iter += strlen(entry->key);
        }
        else
        {
            len += 1;
            iter += 1;
        }
    }

    char *out = ARENA_MALLOC(get_global_arena(), len + 1, "str_replace_many", NULL);

    size_t offset = 0; // offset into input string
    for (size_t i = 0; i < len;)
    {
        const map_entry_t *entry = find_matching_key(pairs, str + offset);
        if (entry != NULL)
        {
            memcpy(out + i, entry->value, strlen(entry->value));
            i += strlen(entry->value);
            offset += strlen(entry->key);
        }
        else
        {
            out[i++] = str[offset++];
        }
    }

    out[len] = '\0';

    return out;
}

USE_DECL
bool str_equal(const char *lhs, const char *rhs)
{
    CTASSERT(lhs != NULL);
    CTASSERT(rhs != NULL);

    /* compare pointers as well for better perf
       with interned strings */
    return lhs == rhs || strcmp(lhs, rhs) == 0;
}

/**
 * modified version of https://rosettacode.org/wiki/Longest_common_prefix#C
 *
 * expects a list of file paths
 */
USE_DECL
const char *str_common_prefix(vector_t *args)
{
    CTASSERT(args != NULL);

    size_t len = vector_len(args);
    CTASSERT(len > 0);

    const char *result = "";

    // only one argument, return it
    if (len == 1)
    {
        const char *it = vector_get(args, 0);
        CTASSERT(it != NULL);
        return it;
    }

    arena_t *arena = get_global_arena();

    size_t size = len * sizeof(char *);
    char **strings = ARENA_MALLOC(arena, size, "str_common_prefix", NULL);

    size_t lower = SIZE_MAX;

    // find common prefix, we account for path seperators
    // because this is used to shrink file paths
    for (size_t i = 0; i < len; i++)
    {
        char *arg = vector_get(args, i);
        CTASSERTF(arg != NULL, "args[%zu] = NULL", i);

        // find the last path seperator
        // we find the common prefix up to the last path seperator
        size_t find = str_rfind_any(arg, PATH_SEPERATORS) + 1;
        strings[i] = ctu_strndup(arg, find);

        lower = MIN(lower, find);
    }

    // no common prefix was found
    if (lower == 0 || lower == SIZE_MAX)
    {
        goto finish;
    }

    for (size_t i = 0; i < lower; i++)
    {
        for (size_t j = 1; j < len; j++)
        {
            if (strings[j][i] != strings[0][i])
            {
                result = i == 0 ? "" : ctu_strndup(strings[0], i);
                goto finish;
            }
        }
    }

    // if we get here, the common prefix is the shortest string
    result = ctu_strndup(strings[0], lower);

finish:
    CTASSERT(strings != NULL);
    for (size_t i = 0; i < len; i++)
    {
        CTASSERTF(strings[i] != NULL, "strings[%zu] = NULL", i);
        arena_free(strings[i], ALLOC_SIZE_UNKNOWN, arena);
    }

    arena_free(strings, size, arena);

    CTASSERT(result != NULL);
    return result;
}

static size_t str_rfind_inner(const char *str, size_t len, const char *sub, size_t sublen)
{
    CTASSERT(str != NULL);
    CTASSERT(sub != NULL);

    if (len == 0) { return SIZE_MAX; }

    CTASSERTM(sublen > 0, "sub must be non-empty");

    while (len--)
    {
        if (strncmp(str + len, sub, sublen) == 0)
        {
            return len;
        }
    }

    return SIZE_MAX;
}

USE_DECL
size_t str_rfind(const char *str, const char *sub)
{
    CTASSERT(str != NULL);
    CTASSERT(sub != NULL);

    size_t len = strlen(str);
    size_t sublen = strlen(sub);

    return str_rfind_inner(str, len, sub, sublen);
}

USE_DECL
size_t str_rfindn(const char *str, size_t len, const char *sub)
{
    CTASSERT(str != NULL);
    CTASSERT(sub != NULL);

    size_t sublen = strlen(sub);

    return str_rfind_inner(str, len, sub, sublen);
}

USE_DECL
size_t str_find(const char *str, const char *sub)
{
    CTASSERT(str != NULL);
    CTASSERT(sub != NULL);

    char *ptr = strstr(str, sub);
    return ptr == NULL ? SIZE_MAX : (size_t)(ptr - str);
}

USE_DECL
size_t str_count_any(const char *str, const char *chars)
{
    CTASSERT(str != NULL);
    CTASSERT(chars != NULL);
    CTASSERT(strlen(chars) > 0);

    size_t count = 0;

    for (size_t i = 0; str[i] != '\0'; i++)
    {
        if (char_is_any_of(str[i], chars))
        {
            count++;
        }
    }

    return count;
}

USE_DECL
char *str_trim(const char *str, const char *letters)
{
    CTASSERT(str != NULL);
    CTASSERT(letters != NULL);

    const char *tmp = str;

    // strip from front
    while (*tmp && char_is_any_of(*tmp, letters))
    {
        tmp++;
    }

    size_t remaining = strlen(tmp);

    // strip from back
    while (remaining > 0 && char_is_any_of(tmp[remaining - 1], letters))
    {
        remaining--;
    }

    return ctu_strndup(tmp, remaining);
}

USE_DECL
char *str_erase(const char *str, size_t len, const char *letters)
{
    CTASSERT(str != NULL);
    CTASSERT(letters != NULL);

    char *result = ctu_strndup(str, len);

    size_t used = len;
    for (size_t i = 0; i < used; i++)
    {
        if (char_is_any_of(result[i], letters))
        {
            memcpy(result + i, result + i + 1, len - i);
            used--;
            i--;
        }
    }

    return result;
}

USE_DECL
char *str_upper(const char *str)
{
    CTASSERT(str != NULL);

    char *result = ctu_strdup(str);
    char *temp = result;

    while (*temp)
    {
        *temp = (char)toupper(*temp);
        temp += 1;
    }

    return result;
}

USE_DECL
char *str_lower(const char *str)
{
    CTASSERT(str != NULL);

    char *result = ctu_strdup(str);
    char *temp = result;

    while (*temp)
    {
        *temp = str_tolower(*temp);
        temp += 1;
    }

    return result;
}

USE_DECL
char str_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return (char)((c + 'a' - 'A') & CHAR_MAX);
    }

    if (CHAR_MIN <= c && c <= CHAR_MAX)
    {
        return (char)c;
    }

    return '\0';
}