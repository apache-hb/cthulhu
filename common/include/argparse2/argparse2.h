#pragma once

#include <stdbool.h>

typedef struct ap2_t ap2_t;
typedef struct ap2_param_t ap2_param_t;
typedef struct ap2_group_t ap2_group_t;

typedef enum ap2_event_result_t
{
    eEventHandled,
    eEventContinue,
    
    eEventTotal
} ap2_event_result_t;

typedef ap2_event_result_t (*ap2_event_t)(ap2_t *ap2, ap2_param_t *param, void *data);

ap2_t *ap2_new(void);

ap2_group_t *ap2_group_new(
    ap2_t *self, 
    const char *name, 
    const char *desc
);

ap2_param_t *ap2_param_bool(
    const char *desc, 
    const char **names,
    bool defaultValue
);

ap2_param_t *ap2_param_int(
    const char *desc, 
    const char **names,
    int defaultValue
);

ap2_param_t *ap2_param_string(
    const char *desc, 
    const char **names,
    const char *defaultValue
);

void ap2_add(ap2_group_t *self, ap2_param_t *param);

/**
 * @brief add a callback event to a parameter
 * 
 * @param self the parser instance
 * @param param the parameter to add the event to, or NULL to use for positional args
 * @param callback 
 * @param data 
 */
void ap2_event(ap2_t *self, ap2_param_t *param, ap2_event_t callback, void *data);
