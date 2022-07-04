#pragma once

#include "analyze.h"

#if ENABLE_TUNING
#    include <stdatomic.h>

BEGIN_API

/**
 * @defgroup Tuning Memory allocation statistics
 * @brief Memory allocation statistics for tuning
 *
 * when ENABLE_TUNING is defined, memory allocation statistics are collected.
 * these are only collected for calls to @ref ctu_malloc and @ref ctu_realloc.
 * hence these functions should be used instead of their stdlib equivalents.
 * @{
 */

typedef atomic_size_t count_t; ///< atomic memory counter type

/**
 * allocation statistics
 */
typedef struct
{
    count_t mallocs;  ///< calls to malloc
    count_t reallocs; ///< calls to realloc
    count_t frees;    ///< calls to free

    count_t current; ///< current memory allocated
    count_t peak;    ///< peak memory allocated
} counters_t;

/**
 * @brief get the current memory allocation statistics
 *
 * @return the current memory allocation statistics
 */
counters_t get_counters(void);

/**
 * @brief get the current memory allocation statistics and reset them
 *
 * @return the current memory allocation statistics
 */
counters_t reset_counters(void);

END_API

/** @} */
#endif
