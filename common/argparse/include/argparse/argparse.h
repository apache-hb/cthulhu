#pragma once

#include "core/analyze.h"
#include "core/version-def.h"

#include "argparse/argparse.h"

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
/// @{

typedef struct reports_t reports_t;
typedef struct node_t node_t;
typedef struct vector_t vector_t;

typedef struct ap_t ap_t;
typedef struct ap_param_t ap_param_t;
typedef struct ap_group_t ap_group_t;

/// @brief the continuation code of a user event
typedef enum ap_event_result_t
{
    eEventHandled, ///< the event was handled, dont find the next handler
    eEventContinue, ///< the event was not handled by this handler, find the next handler

    eEventTotal
} ap_event_result_t;

/// @brief callback for a parameter event
///
/// @param ap the parser instance
/// @param param the parameter that triggered the event, may be NULL for positional args
/// @param value the value of the parameter, may be NULL for positional args
///              is mpz_t for int, const char * for string, and bool* for bool
/// @param data the data passed to @see ap_event
///
/// @return continuation code
typedef ap_event_result_t (*ap_event_t)(ap_t *ap, const ap_param_t *param, const void *value, void *data);

/// @brief callback for an error event
///
/// @param ap the parser instance
/// @param node the node that triggered the error
/// @param message the error message
/// @param data the data passed to @see ap_error
///
/// @return continuation code
typedef ap_event_result_t (*ap_error_t)(ap_t *ap, const node_t *node, const char *message, void *data);

// initialization + config api

/// @brief create a new parser instance
///
/// @param desc the description of the program
/// @param version the version info of the program
///
/// @return the created parser instance
ap_t *ap_new(const char *desc, version_t version);

/// @brief add a group to the parser
///
/// @param self the parser instance
/// @param name the name of the group
/// @param desc the description of the group
///
/// @return the created group
ap_group_t *ap_group_new(ap_t *self, const char *name, const char *desc);

/// @brief add a bool parameter to a group
///
/// @param self the group to add to
/// @param name the name of the parameter
/// @param desc the description of the parameter
/// @param names the names of the parameter, must be NULL terminated
///
/// @return the created parameter
ap_param_t *ap_add_bool(ap_group_t *self, const char *name, const char *desc, const char **names);

/// @brief add an int parameter to a group
///
/// @param self the group to add to
/// @param name the name of the parameter
/// @param desc the description of the parameter
/// @param names the names of the parameter, must be NULL terminated
///
/// @return the created parameter
ap_param_t *ap_add_int(ap_group_t *self, const char *name, const char *desc, const char **names);

/// @brief add a string parameter to a group
///
/// @param self the group to add to
/// @param name the name of the parameter
/// @param desc the description of the parameter
/// @param names the names of the parameter, must be NULL terminated
///
/// @return the created parameter
ap_param_t *ap_add_string(ap_group_t *self, const char *name, const char *desc, const char **names);

/// @brief get a boolean value from a parameter
///
/// @param self the parser instance
/// @param param the parameter to get the value from
/// @param[out] value the value to set
///
/// @return true if the value was set
bool ap_get_bool(ap_t *self, const ap_param_t *param, bool *value);

/// @brief get an integer value from a parameter
///
/// @param self the parser instance
/// @param param the parameter to get the value from
/// @param[out] value the value to set
///
/// @return true if the value was set
bool ap_get_int(ap_t *self, const ap_param_t *param, mpz_t value);

/// @brief get a string value from a parameter
///
/// @param self the parser instance
/// @param param the parameter to get the value from
/// @param[out] value the value to set
///
/// @return true if the value was set
bool ap_get_string(ap_t *self, const ap_param_t *param, const char **value);

/// @brief add a callback event to a parameter
///
/// @param self the parser instance
/// @param param the parameter to add the event to
/// @param callback the callback to add
/// @param data the data to pass to the callback
void ap_event(ap_t *self, ap_param_t *param, ap_event_t callback, void *data);

/// @brief add a callback error event to the parser
/// @note this is called when an error occurs in the parser
///
/// @param self the parser instance
/// @param callback the callback to add
/// @param data the data to pass to the callback
void ap_error(ap_t *self, ap_error_t callback, void *data);

// TODO: remove reports here, requires a refactor of the reports module

/// @brief parse a command line or other string
///
/// @param self the parser instance
/// @param reports reporting channel
/// @param argc from main
/// @param argv from main
///
/// @return int exit code
int ap_parse(ap_t *self, reports_t *reports, int argc, const char **argv);

/// @defgroup ArgParseReflection Argument parsing reflection
/// @brief functions for getting information about the parser
/// @{

/// @brief get all currently registered groups in the parser
///
/// @param self the parser instance
/// @return all currently registered groups
NODISCARD CONSTFN
const vector_t *ap_get_groups(const ap_t *self);

/// @brief get all currently registered parameters in a group
///
/// @param self the group instance
/// @return all currently registered parameters
NODISCARD CONSTFN
const vector_t *ap_get_params(const ap_group_t *self);

/// @} // ArgParseReflection

/// @} // ArgParse

END_API