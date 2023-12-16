#pragma once

#include "notify/notify.h"
#include "notify/text.h"

const char *get_severity_name(severity_t severity);
const char *get_severity_colour(text_colour_t colours, severity_t severity);

const char *get_scan_name(const node_t *node);
