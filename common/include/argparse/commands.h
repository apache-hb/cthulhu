#pragma once

#include "argparse/argparse.h"

#define AP_EVENT(name, ap, param, value, data) \
    ap_event_result_t name(ap_t *ap, const ap_param_t *param, const void *value, void *data)

#define AP_ERROR(name, ap, node, message, data) \
    ap_event_result_t name(ap_t *ap, const node_t *node, const char *message, void *data)

void ap_help(const ap_t *ap, const char *name);
void ap_version(const ap_t *ap);
