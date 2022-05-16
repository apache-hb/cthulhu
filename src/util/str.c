#include "cthulhu/util/str.h"
#include "cthulhu/util/defs.h"
#include "cthulhu/util/util.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char *format(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *str = formatv(fmt, args);
    va_end(args);

    return str;
}

char *formatv(const char *fmt, va_list args)
{
    /* make a copy of the args for the second format */
    va_list again;
    va_copy(again, args);

    /* get the number of bytes needed to format */
    int len = vsnprintf(NULL, 0, fmt, args) + 1;

    char *out = ctu_malloc(len);

    vsnprintf(out, len, fmt, again);

    va_end(again);

    return out;
}

char *str_noext(const char *path)
{
    size_t idx = str_rfind(path, ".");
    char *base = ctu_strdup(path);
    if (idx == SIZE_MAX)
    {
        return base;
    }

    base[idx] = '\0';
    return base;
}

char *str_filename(const char *path)
{
    size_t idx = str_rfind(path, PATH_SEP);
    if (idx == SIZE_MAX)
    {
        return str_noext(path);
    }

    return str_noext(path + idx + 1);
}

bool str_startswith(const char *str, const char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool str_endswith(const char *str, const char *suffix)
{
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
    {
        return false;
    }

    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

char *str_join(const char *sep, vector_t *parts)
{
    size_t all = vector_len(parts);

    if (all == 0)
    {
        return ctu_strdup("");
    }

    if (all == 1)
    {
        return vector_get(parts, 0);
    }

    size_t len = 0;
    size_t seplen = strlen(sep);
    for (size_t i = 0; i < all; i++)
    {
        const char *part = vector_get(parts, i);
        len += strlen(part);

        if (i != 0)
        {
            len += seplen;
        }
    }

    char *out = ctu_malloc(len + 1);
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
        size_t partLength = strlen(part);
        memcpy(out + idx, part, MIN(remaining, partLength));
        idx += partLength;
        remaining -= partLength;
    }

    out[idx] = 0;
    return out;
}

char *str_repeat(const char *str, size_t times)
{
    size_t len = strlen(str);
    size_t outlen = len * times;
    char *out = ctu_malloc(outlen + 1);
    size_t remaining = outlen;
    for (size_t i = 0; i < times; i++)
    {
        memcpy(out + i * len, str, MIN(remaining, len));
        remaining -= len;
    }
    out[outlen] = 0;
    return out;
}

static bool safe_isprint(char c)
{
    if (c == '\n' || c == '\t' || c == '\f' || c == '\r' || c == '\v')
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
    switch (c)
    {
    case '\0':
    case '\\':
    case '\'':
    case '\"':
        return 2;
    default:
        return safe_isprint(c) ? 1 : 4;
    }
}

static size_t normstr(char *out, char c)
{
    if (c == '\\')
    {
        out[0] = '\\';
        out[1] = '\\';
        return 2;
    }

    if (c == '\'')
    {
        out[0] = '\\';
        out[1] = '\'';
        return 2;
    }

    if (c == '\"')
    {
        out[0] = '\\';
        out[1] = '\"';
        return 2;
    }

    if (c == '\0')
    {
        out[0] = '\\';
        out[1] = '0';
        return 2;
    }

    if (safe_isprint(c))
    {
        out[0] = c;
        return 1;
    }

    return snprintf(out, 5, "\\x%02x", c & 0xFF);
}

char *str_normalize(const char *str)
{
    size_t len = 0;
    const char *temp = str;
    while (*temp != '\0')
    {
        len += normlen(*temp++);
    }

    char *buf = ctu_malloc(len + 1);
    char *out = buf;
    while (*str != '\0')
    {
        out += normstr(out, *str++);
    }
    *out = '\0';

    return buf;
}

USE_DECL
char *str_normalizen(const char *str, size_t len)
{
    size_t outlen = 1;
    size_t actual = 0;
    for (; actual < len; actual++)
    {
        outlen += normlen(str[actual]);
    }

    char *buf = ctu_malloc(outlen + 1);
    char *out = buf;
    for (size_t i = 0; i < actual; i++)
    {
        out += normstr(out, str[i]);
    }

    *out = '\0';
    return buf;
}

vector_t *str_split(const char *str, const char *sep)
{
    size_t seplen = strlen(sep);
    vector_t *result = vector_new(4);

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

        vector_push(&result, ctu_strndup(token, cursor - token));
        token = cursor + seplen;
        cursor += seplen;
    }

    vector_push(&result, ctu_strndup(token, cursor - token));

    return result;
}

size_t strhash(const char *str)
{
    size_t hash = 0;

    while (*str)
    {
        // hash = 31 * hash + (*str++ & 0xff);
        hash = (hash << 5) - hash + *str++;
    }

    return hash;
}

bool str_contains(const char *str, const char *sub)
{
    return strstr(str, sub) != NULL;
}

char *str_replace(const char *str, const char *sub, const char *repl)
{
    vector_t *split = str_split(str, sub);
    return str_join(repl, split);
}

bool str_equal(const char *lhs, const char *rhs)
{
    /* compare pointers as well for better perf
       with interned strings */
    return lhs == rhs || strcmp(lhs, rhs) == 0;
}

/**
 * modified version of https://rosettacode.org/wiki/Longest_common_prefix#C
 *
 * expects a list of file paths
 */
const char *common_prefix(vector_t *args)
{
    size_t len = vector_len(args);
    CTASSERT(len > 0, "must have at least one argument");

    if (len == 1)
    {
        return vector_get(args, 0);
    }

    char **strings = ctu_malloc(len * sizeof(char *));

    size_t lower = SIZE_MAX;

    for (size_t i = 0; i < len; i++)
    {
        char *arg = vector_get(args, i);
        size_t find = str_rfind(arg, PATH_SEP) + 1;
        strings[i] = ctu_strndup(arg, find);

        lower = MIN(lower, find);
    }

    if (lower == 0 || lower == SIZE_MAX)
    {
        return "";
    }

    for (size_t i = 0; i < lower; i++)
    {
        for (size_t j = 1; j < len; j++)
        {
            if (strings[j][i] != strings[0][i])
            {
                return i == 0 ? "" : ctu_strndup(strings[0], i);
            }
        }
    }

    return ctu_strndup(strings[0], lower);
}

size_t str_find(const char *str, const char *sub)
{
    size_t len = strlen(str);
    size_t sublen = strlen(sub);

    CTASSERT(len > 0, "str must be non-empty");
    CTASSERT(sublen > 0, "sub must be non-empty");

    if (sublen > len)
    {
        return SIZE_MAX;
    }

    for (size_t i = 0; i < len - sublen; i++)
    {
        if (strncmp(str + i, sub, sublen) == 0)
        {
            return i;
        }
    }

    return SIZE_MAX;
}

static size_t str_rfind_inner(const char *str, size_t len, const char *sub, size_t sublen)
{
    CTASSERT(len > 0, "str must be non-empty");
    CTASSERT(sublen > 0, "sub must be non-empty");

    while (len--)
    {
        if (strncmp(str + len, sub, sublen) == 0)
        {
            return len;
        }
    }

    return SIZE_MAX;
}

size_t str_rfind(const char *str, const char *sub)
{
    size_t len = strlen(str);
    size_t sublen = strlen(sub);

    return str_rfind_inner(str, len, sub, sublen);
}

USE_DECL
size_t str_rfindn(const char *str, size_t len, const char *sub)
{
    size_t sublen = strlen(sub);

    return str_rfind_inner(str, len, sub, sublen);
}

static bool char_is_any_of(char c, const char *chars)
{
    for (; *chars; chars++)
    {
        if (*chars == c)
        {
            return true;
        }
    }

    return false;
}

char *str_trim(const char *str, const char *chars)
{
    const char *tmp = str;

    // strip from front
    while (*tmp && char_is_any_of(*tmp, chars))
    {
        tmp++;
    }

    size_t remaining = strlen(tmp);

    // strip from back
    while (remaining > 0 && char_is_any_of(tmp[remaining - 1], chars))
    {
        remaining--;
    }

    return ctu_strndup(tmp, remaining);
}
