#pragma once

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
    const char **names
);

ap2_param_t *ap2_param_int(
    const char *desc, 
    const char **names
);

ap2_param_t *ap2_param_string(
    const char *desc, 
    const char **names
);

void ap2_add(ap2_group_t *self, ap2_param_t *param);
void ap2_event(ap2_t *self, ap2_param_t *param, ap2_event_t callback, void *data);
