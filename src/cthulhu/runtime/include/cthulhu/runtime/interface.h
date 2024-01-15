#pragma once

#include "cthulhu/runtime/common.h"

BEGIN_API

typedef struct io_t io_t;
typedef struct vector_t vector_t;
typedef struct map_t map_t;

/// @defgroup mediator Compiler core interface
/// @brief Core of the compiler, manages stages and language agnostic data
/// @ingroup runtime
/// @{

/// @brief create the global mediator object, must be the first part of cthulhu called
///
/// @param arena the allocator to use
///
/// @return mediator_t* the mediator object
CT_RUNTIME_API mediator_t *mediator_new(IN_NOTNULL arena_t *arena);

// lifetime api

/**
 * @brief create a new lifetime object
 *
 * @param mediator the global object
 * @param arena the allocator to use
 * @return lifetime_t* the lifetime object
 */
CT_RUNTIME_API lifetime_t *lifetime_new(IN_NOTNULL mediator_t *mediator, IN_NOTNULL arena_t *arena);

/**
 * @brief add a new language to the lifetime object
 *
 * @param lifetime the lifetime object
 * @param lang the language object
 */
CT_RUNTIME_API void lifetime_add_language(IN_NOTNULL lifetime_t *lifetime, IN_NOTNULL const language_t *lang);

/**
 * @brief map a file extension to a language driver
 *
 * @param lifetime the lifetime object
 * @param ext the file extension not including the dot
 * @param lang the language object
 * @return const language_t* the previous language object mapped to the extension, or NULL
 */
CT_RUNTIME_API const language_t *lifetime_add_extension(IN_NOTNULL lifetime_t *lifetime, IN_STRING const char *ext, IN_NOTNULL const language_t *lang);

/**
 * @brief get the language object for a file extension
 *
 * @param lifetime the lifetime object
 * @param ext the file extension not including the dot
 * @return const language_t* the language object, or NULL
 */
CT_RUNTIME_API const language_t *lifetime_get_language(IN_NOTNULL lifetime_t *lifetime, IN_STRING const char *ext);

/**
 * @brief parse an io object using a language driver
 *
 * @param lifetime the lifetime object
 * @param lang the language object
 * @param io the io object
 */
CT_RUNTIME_API void lifetime_parse(IN_NOTNULL lifetime_t *lifetime, IN_NOTNULL const language_t *lang, IN_NOTNULL io_t *io);

/**
 * @brief run a stage on a lifetime object
 * @note stages must be run in order
 *
 * @param lifetime the lifetime object
 * @param stage the stage to run
 */
CT_RUNTIME_API void lifetime_run_stage(IN_NOTNULL lifetime_t *lifetime, compile_stage_t stage);

/**
 * @brief resolve all dependencies in a lifetime object
 * @note must be called after all stages have been run
 * @param lifetime the lifetime object
 */
CT_RUNTIME_API void lifetime_resolve(IN_NOTNULL lifetime_t *lifetime);

/**
 * @brief get all compiled modules from a lifetime object
 *
 * @param lifetime the lifetime object
 * @return map_t* a map of module names to module objects
 */
CT_RUNTIME_API map_t *lifetime_get_modules(IN_NOTNULL lifetime_t *lifetime);

/// @}

END_API
