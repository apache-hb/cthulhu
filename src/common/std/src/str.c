#include "std/str.h"

#include "std/map.h"
#include "std/typed/vector.h"
#include "std/vector.h"

#include "arena/arena.h"

#include "base/util.h"
#include "base/panic.h"

#include "core/macros.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

USE_DECL
text_t text_format(arena_t *arena, const char *fmt, ...)
{
    CTASSERT(arena != NULL);
    CTASSERT(fmt != NULL);

    va_list args;
    va_start(args, fmt);

    text_t text = text_vformat(arena, fmt, args);

    va_end(args);

    return text;
}

USE_DECL
text_t text_vformat(arena_t *arena, const char *fmt, va_list args)
{
    CTASSERT(arena != NULL);
    CTASSERT(fmt != NULL);

    // make a copy of the args for the second format
    va_list again;
    va_copy(again, args);

    // get the number of bytes needed to format
    int len = vsnprintf(NULL, 0, fmt, args);

    CTASSERTF(len > 0, "text_vformat failed to format string: %s", fmt);

    char *out = ARENA_MALLOC(len + 1, "text_vformat", fmt, arena);

    int result = vsnprintf(out, len + 1, fmt, again);
    CTASSERTF(result == len, "text_vformat failed to format string: %s expected (%d == %d)", fmt, result, len);

    va_end(again);

    return text_make(out, len);
}

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

    text_t text = text_vformat(arena, fmt, args);

    return text.text;
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

    size_t i = ctu_strlen(str);
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
char *str_noext(const char *path, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);

    size_t idx = str_rfind(path, ".");
    char *base = arena_strdup(path, arena);
    if (idx == SIZE_MAX)
    {
        return base;
    }

    base[idx] = '\0';
    return base;
}

USE_DECL
char *str_ext(const char *path, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);

    size_t idx = str_rfind(path, ".");
    if (idx == SIZE_MAX)
    {
        return NULL;
    }

    return arena_strdup(path + idx + 1, arena);
}

USE_DECL
char *str_directory(const char *path, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);

    size_t idx = str_rfind_any(path, PATH_SEPERATORS);
    if (idx == SIZE_MAX)
    {
        return arena_strdup(".", arena);
    }

    return arena_strndup(path, idx, arena);
}

USE_DECL
char *str_basename(const char *path, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);

    size_t idx = str_rfind_any(path, PATH_SEPERATORS);
    if (idx == SIZE_MAX)
    {
        return str_noext(path, arena);
    }

    return str_noext(path + idx + 1, arena);
}

USE_DECL
char *str_filename(const char *path, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);

    size_t idx = str_rfind_any(path, PATH_SEPERATORS);
    if (idx == SIZE_MAX)
    {
        return arena_strdup(path, arena);
    }

    return arena_strdup(path + idx + 1, arena);
}

USE_DECL
bool str_startswith(const char *str, const char *prefix)
{
    CTASSERT(str != NULL);
    CTASSERT(prefix != NULL);

    return ctu_strncmp(str, prefix, ctu_strlen(prefix)) == 0;
}

USE_DECL
bool str_endswith(const char *str, const char *suffix)
{
    CTASSERT(str != NULL);
    CTASSERT(suffix != NULL);

    size_t lenstr = ctu_strlen(str);
    size_t lensuffix = ctu_strlen(suffix);
    if (lensuffix > lenstr)
    {
        return false;
    }

    return ctu_strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

USE_DECL
char *str_join(const char *sep, vector_t *parts, arena_t *arena)
{
    CTASSERT(sep != NULL);
    CTASSERT(parts != NULL);
    CTASSERT(arena != NULL);

    size_t all = vector_len(parts);

    if (all == 0)
    {
        return arena_strndup("", 0, arena);
    }
    else if (all == 1)
    {
        char *it = vector_get(parts, 0);
        CTASSERT(it != NULL);
        return arena_strdup(it, arena);
    }

    size_t len = 0;
    size_t seplen = ctu_strlen(sep);
    for (size_t i = 0; i < all; i++)
    {
        const char *part = vector_get(parts, i);
        CTASSERTF(part != NULL, "part[%zu] = NULL", i);

        len += ctu_strlen(part);

        if (i != 0)
        {
            len += seplen;
        }
    }

    CTASSERTF(len > 0, "len = %zu", len);

    char *out = ARENA_MALLOC(len + 1, "str_join", parts, arena);
    size_t idx = 0;

    size_t remaining = len;
    for (size_t i = 0; i < all; i++)
    {
        if (i != 0)
        {
            ctu_memcpy(out + idx, sep, MIN(remaining, seplen));
            idx += seplen;
            remaining -= seplen;
        }

        const char *part = vector_get(parts, i);
        size_t part_len = ctu_strlen(part);
        ctu_memcpy(out + idx, part, MIN(remaining, part_len));
        idx += part_len;
        remaining -= part_len;
    }

    out[idx] = 0;
    return out;
}

USE_DECL
char *str_repeat(const char *str, size_t times, arena_t *arena)
{
    CTASSERT(str != NULL);
    CTASSERT(arena != NULL);

    if (times == 0)
    {
        return arena_strdup("", arena);
    }

    size_t len = ctu_strlen(str);
    size_t outlen = len * times;
    char *out = ARENA_MALLOC(outlen + 1, "str_repeat", str, arena);
    size_t remaining = outlen;
    for (size_t i = 0; i < times; i++)
    {
        ctu_memcpy(out + i * len, str, MIN(remaining, len));
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

static bool ctu_isprint(char c)
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
    return ctu_isprint(c) ? 1 : 4;
}

static size_t normstr(char *out, char c)
{
    if (ctu_isprint(c))
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
char *str_normalize(const char *input, arena_t *arena)
{
    CTASSERT(input != NULL);
    CTASSERT(arena != NULL);

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
        return arena_strndup(input, input_length, arena);
    }

    const char *repl_iter = input;
    char *buf = ARENA_MALLOC(result_length + 1, "str_normalize", input, arena);
    char *result = buf;
    while (*repl_iter != '\0')
    {
        result += normstr(result, *repl_iter++);
    }
    *result = '\0';

    return buf;
}

USE_DECL
char *str_normalizen(text_view_t text, arena_t *arena)
{
    CTASSERT(text.text != NULL);
    CTASSERT(arena != NULL);

    const char *str = text.text;
    size_t len = text.length;

    size_t length = 1;
    for (size_t i = 0; i < len; i++)
    {
        length += normlen(str[i]);
    }

    // if the string is already normalized, just return a copy
    if (length == len)
    {
        return arena_strndup(str, len, arena);
    }

    char *buf = ARENA_MALLOC(length + 1, "str_normalizen", str, arena);
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
vector_t *str_split(IN_STRING const char *str, IN_STRING const char *sep, arena_t *arena)
{
    CTASSERT(str != NULL);
    CTASSERT(sep != NULL);
    CTASSERT(arena != NULL);

    if (ctu_strlen(sep) == 0)
    {
        // split into individual characters
        vector_t *result = vector_new(ctu_strlen(str), arena);
        for (size_t i = 0; i < ctu_strlen(str); i++)
        {
            char *c = ARENA_MALLOC(2, "str_split", str, arena);
            c[0] = str[i];
            c[1] = '\0';
            vector_push(&result, c);
        }
        return result;
    }

    size_t seplen = ctu_strlen(sep);
    vector_t *result = vector_new(4, arena);

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
size_t str_hash(const char *str)
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
size_t text_hash(text_view_t text)
{
    CTASSERT(text.text != NULL);

    size_t hash = 0;
    for (size_t i = 0; i < text.length; i++)
    {
        hash = (hash << 5) - hash + text.text[i];
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
char *str_replace(const char *str, const char *search, const char *repl, arena_t *arena)
{
    CTASSERT(str != NULL);
    CTASSERT(search != NULL);
    CTASSERT(repl != NULL);

    if (ctu_strlen(search) == 0)
    {
        return arena_strdup(str, arena);
    }

    vector_t *split = str_split(str, search, arena);
    return str_join(repl, split, arena);
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
char *str_replace_many(const char *str, const map_t *repl, arena_t *arena)
{
    CTASSERT(str != NULL);
    CTASSERT(repl != NULL);
    CTASSERT(arena != NULL);

    size_t len = 0;
    typevec_t *pairs = map_entries((map_t*)repl); // TODO: map_entries should have a const version

    const char *iter = str;
    while (*iter)
    {
        const map_entry_t *entry = find_matching_key(pairs, iter);
        if (entry != NULL)
        {
            len += ctu_strlen(entry->value);
            iter += ctu_strlen(entry->key);
        }
        else
        {
            len += 1;
            iter += 1;
        }
    }

    char *out = ARENA_MALLOC(len + 1, "str_replace_many", repl, arena);

    size_t offset = 0; // offset into input string
    for (size_t i = 0; i < len;)
    {
        const map_entry_t *entry = find_matching_key(pairs, str + offset);
        if (entry != NULL)
        {
            ctu_memcpy(out + i, entry->value, ctu_strlen(entry->value));
            i += ctu_strlen(entry->value);
            offset += ctu_strlen(entry->key);
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
const char *str_common_prefix(vector_t *args, arena_t *arena)
{
    CTASSERT(args != NULL);
    CTASSERT(arena != NULL);

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

    size_t size = len * sizeof(char *);
    char **strings = ARENA_MALLOC(size, "str_common_prefix", NULL, arena);

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
        strings[i] = arena_strndup(arg, find, arena);

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
                result = i == 0 ? "" : arena_strndup(strings[0], i, arena);
                goto finish;
            }
        }
    }

    // if we get here, the common prefix is the shortest string
    result = arena_strndup(strings[0], lower, arena);

finish:
    CTASSERT(strings != NULL);
    for (size_t i = 0; i < len; i++)
    {
        CTASSERTF(strings[i] != NULL, "strings[%zu] = NULL", i);
        arena_free(strings[i], CT_ALLOC_SIZE_UNKNOWN, arena);
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
        if (ctu_strncmp(str + len, sub, sublen) == 0)
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

    size_t len = ctu_strlen(str);
    size_t sublen = ctu_strlen(sub);

    return str_rfind_inner(str, len, sub, sublen);
}

USE_DECL
size_t str_find(const char *str, const char *sub)
{
    CTASSERT(str != NULL);
    CTASSERT(sub != NULL);

    const char *ptr = strstr(str, sub);
    return ptr == NULL ? SIZE_MAX : (size_t)(ptr - str);
}

USE_DECL NOALIAS
char *str_erase(char *str, size_t len, const char *letters)
{
    CTASSERT(str != NULL);
    CTASSERT(letters != NULL);

    // start at the back of str and remove letters,
    // if a letter is removed then the offset doesnt get updated
    // if a letter isnt removed then the offset gets updated
    // push any characters that arent removed to the offset and update it

    size_t remaining = len;
    size_t offset = len;
    while (remaining > 0)
    {
        if (char_is_any_of(str[remaining - 1], letters))
        {
            remaining--;
        }
        else
        {
            str[--offset] = str[--remaining];
        }
    }

    return str + offset;
}

USE_DECL
char *str_upper(const char *str, arena_t *arena)
{
    CTASSERT(str != NULL);
    CTASSERT(arena != NULL);

    char *result = arena_strdup(str, arena);
    char *temp = result;

    while (*temp)
    {
        *temp = str_toupper(*temp);
        temp += 1;
    }

    return result;
}

USE_DECL
char *str_lower(const char *str, arena_t *arena)
{
    CTASSERT(str != NULL);
    CTASSERT(arena != NULL);

    char *result = arena_strdup(str, arena);
    char *temp = result;

    while (*temp)
    {
        *temp = str_tolower(*temp);
        temp += 1;
    }

    return result;
}

USE_DECL
char str_tolower(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return (char)((c + 'a' - 'A') & CHAR_MAX);
    }

    return c;
}

USE_DECL
char str_toupper(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return (char)((c + 'A' - 'a') & CHAR_MAX);
    }

    return c;
}
