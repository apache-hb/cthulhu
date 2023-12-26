#pragma once

#include "core/compiler.h"

#include <gmp.h>
#include <stdbool.h>

BEGIN_API

/**
 * argparse control flow
 *
 * parser instance
 *   - contains a list of groups
 *   - each group contains a list of params
 *   - params may not share names
 *
 * when parsing:
 *  - when a param is encountered, its related events are called
 *  - posargs are sent to a global event
 *  - each event is part of its chain, simillar to windows driver filters
 *
 * parameters can be added while the parser is running
 *   - this is how modules are dynamically loaded
 *
 */

/// @defgroup ArgParse Command line argument parsing
/// @brief Command line argument parsing
/// @ingroup Runtime
/// @{

typedef struct node_t node_t;
typedef struct vector_t vector_t;
typedef struct arena_t arena_t;

typedef struct config_t config_t;
typedef struct cfg_field_t cfg_field_t;

typedef struct ap_t ap_t;

/// @brief the continuation code of a user event
typedef enum ap_event_result_t
{
    eEventHandled, ///< the event was handled, dont find the next handler
    eEventContinue, ///< the event was not handled by this handler, find the next handler

    eEventCount
} ap_event_result_t;

typedef enum ap_error_type_t
{
    eErrorUnknownFlag,
    eErrorIntRange,
    eErrorInvalidEnum,

    eErrorCount
} ap_error_type_t;

typedef struct ap_error_t
{
    ap_error_type_t type;

    union {
        const char *unknown_flag;

        struct {
            const cfg_field_t *param;

            union {
                int int_value;
                const char *enum_value;
            };
        };
    };
} ap_error_t;

/// @brief callback for a parameter event
///
/// @param ap the parser instance
/// @param param the parameter that triggered the event, may be NULL for positional args
/// @param value the value of the parameter, may be NULL for positional args
///              is mpz_t for int, const char * for string, and bool* for bool
/// @param data the data passed to @see ap_event
///
/// @return continuation code
typedef ap_event_result_t (*ap_event_t)(ap_t *ap, const cfg_field_t *param, const void *value, void *data);

// initialization + config api

/// @brief create a new parser instance
///
/// @param config the config object to use
/// @param arena the arena to allocate from
///
/// @return the created parser instance
ap_t *ap_new(config_t *config, arena_t *arena);

/// @brief add a callback event to a parameter
///
/// @param self the parser instance
/// @param param the parameter to add the event to
/// @param callback the callback to add
/// @param data the data to pass to the callback
void ap_event(ap_t *self, const cfg_field_t *param, ap_event_t callback, void *data);

/// @brief parse a command line or other string
///
/// @param self the parser instance
/// @param argc from main
/// @param argv from main
///
/// @return int exit code
int ap_parse(ap_t *self, int argc, const char **argv);

/// @brief get all positional arguments
///
/// @param self the parser instance
///
/// @return all positional arguments
vector_t *ap_get_posargs(ap_t *self);

/// @brief get all unknown arguments
///
/// @param self the parser instance
///
/// @return all unknown arguments
vector_t *ap_get_unknown(ap_t *self);

/// @brief get the number of processed arguments
///
/// @param self the parser instance
///
/// @return the number of processed arguments
size_t ap_count_params(ap_t *self);

/// @} // ArgParse

END_API
