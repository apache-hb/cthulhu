#pragma once

#include "cthulhu/runtime/common.h"

BEGIN_API

typedef struct io_t io_t;
typedef struct vector_t vector_t;
typedef struct map_t map_t;

/**
 * @defgroup Mediator Compiler core interface
 * @brief Core of the compiler, manages stages and language agnostic data
 * @ingroup Runtime
 * @{
 */

/// @brief initialize the global runtime object
/// @warning this is called by @a mediator_new, if you call this yourself you promise to not use the mediator
/// @note this may only be called once
///
/// @param arena the allocator to use
void runtime_init(arena_t *arena);

/// @brief create the global mediator object, must be the first part of cthulhu called
///
/// @param arena the allocator to use
///
/// @return mediator_t* the mediator object
mediator_t *mediator_new(arena_t *arena);

// lifetime api

/**
 * @brief create a new lifetime object
 *
 * @param mediator the global object
 * @param arena the allocator to use
 * @return lifetime_t* the lifetime object
 */
lifetime_t *lifetime_new(mediator_t *mediator, arena_t *arena);

/**
 * @brief add a new language to the lifetime object
 *
 * @param lifetime the lifetime object
 * @param lang the language object
 */
void lifetime_add_language(lifetime_t *lifetime, const language_t *lang);

/**
 * @brief map a file extension to a language driver
 *
 * @param lifetime the lifetime object
 * @param ext the file extension not including the dot
 * @param lang the language object
 * @return const language_t* the previous language object mapped to the extension, or NULL
 */
const language_t *lifetime_add_extension(lifetime_t *lifetime, const char *ext, const language_t *lang);

/**
 * @brief get the language object for a file extension
 *
 * @param lifetime the lifetime object
 * @param ext the file extension not including the dot
 * @return const language_t* the language object, or NULL
 */
const language_t *lifetime_get_language(lifetime_t *lifetime, const char *ext);

/**
 * @brief parse an io object using a language driver
 *
 * @param lifetime the lifetime object
 * @param lang the language object
 * @param io the io object
 */
void lifetime_parse(lifetime_t *lifetime, const language_t *lang, io_t *io);

/**
 * @brief run a stage on a lifetime object
 * @note stages must be run in order
 *
 * @param lifetime the lifetime object
 * @param stage the stage to run
 */
void lifetime_run_stage(lifetime_t *lifetime, compile_stage_t stage);

/**
 * @brief resolve all dependencies in a lifetime object
 * @note must be called after all stages have been run
 * @param lifetime the lifetime object
 */
void lifetime_resolve(lifetime_t *lifetime);

/**
 * @brief get all compiled modules from a lifetime object
 *
 * @param lifetime the lifetime object
 * @return map_t* a map of module names to module objects
 */
map_t *lifetime_get_modules(lifetime_t *lifetime);

/** @} */

END_API