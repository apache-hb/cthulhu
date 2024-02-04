#include "std/bitset.h"
#include "arena/arena.h"
#include "base/panic.h"
#include "core/macros.h"

#include <limits.h>
#include <string.h>

typedef unsigned char bitset_word_t;

#define WORD_SIZE (sizeof(bitset_word_t) * CHAR_BIT)

static size_t word_count_from_bits(size_t bits)
{
    return (bits + (WORD_SIZE - 1)) / (WORD_SIZE);
}

// index of the word containing the bit
static size_t word_index(size_t bit)
{
    return bit / WORD_SIZE;
}

// offset of the bit within the word
static size_t word_offset(size_t bit)
{
    return bit % WORD_SIZE;
}

static bitset_word_t *bitset_start(const bitset_t set)
{
    CTASSERT(set.data != NULL);

    return (bitset_word_t*)set.data;
}

static bitset_word_t *word_at(const bitset_t set, size_t index)
{
    return bitset_start(set) + index;
}

USE_DECL
bitset_t bitset_new(size_t bits, arena_t *arena)
{
    CTASSERT(bits > 0);
    CTASSERT(arena != NULL);

    size_t words = word_count_from_bits(bits);
    void *data = ARENA_MALLOC(words, "bitset", NULL, arena);

    return bitset_of(data, words);
}

USE_DECL
bitset_t bitset_of(void *data, size_t words)
{
    CTASSERT(data != NULL);
    CTASSERT(words > 0);

    bitset_t bitset = {
        .words = words,
        .data = data
    };

    return bitset;
}

USE_DECL
bool bitset_any(const bitset_t set, const bitset_t mask)
{
    size_t words = CT_MIN(set.words, mask.words);

    bitset_word_t *data = bitset_start(set);
    bitset_word_t *mask_data = bitset_start(mask);

    for (size_t i = 0; i < words; i++)
    {
        if ((data[i] & mask_data[i]) != 0)
        {
            return true;
        }
    }

    return false;
}

USE_DECL
bool bitset_all(const bitset_t set, const bitset_t mask)
{
    size_t words = CT_MIN(set.words, mask.words);

    bitset_word_t *data = bitset_start(set);
    bitset_word_t *mask_data = bitset_start(mask);

    for (size_t i = 0; i < words; i++)
    {
        if ((data[i] & mask_data[i]) != mask_data[i])
        {
            return false;
        }
    }

    return true;
}

USE_DECL
bool bitset_test(const bitset_t set, size_t index)
{
    CTASSERTF(bitset_len(set) > index, "index %zu is out of range %zu", index, bitset_len(set));

    size_t word = word_index(index);
    size_t offset = word_offset(index);

    return (*word_at(set, word) & (1 << offset)) != 0;
}

USE_DECL
void bitset_set(bitset_t set, size_t index)
{
    CTASSERTF(bitset_len(set) > index, "index %zu is out of range %zu", index, bitset_len(set));

    size_t word = word_index(index);
    size_t offset = word_offset(index);

    *word_at(set, word) |= (1 << offset);
}

USE_DECL
void bitset_clear(bitset_t set, size_t index)
{
    CTASSERTF(bitset_len(set) > index, "index %zu is out of range %zu", index, bitset_len(set));

    size_t word = word_index(index);
    size_t offset = word_offset(index);

    *word_at(set, word) &= ~(1 << offset);
}

USE_DECL
void bitset_reset(bitset_t set)
{
    bitset_word_t *data = bitset_start(set);
    memset(data, 0, set.words);
}

USE_DECL
size_t bitset_len(const bitset_t set)
{
    CTASSERT(set.data != NULL);

    return set.words * WORD_SIZE;
}
