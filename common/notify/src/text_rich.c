#include "notify/text.h"

#include "base/panic.h"

void text_report_rich(text_config_t config, const event_t *event)
{
    CTASSERT(config.io != NULL);
    CTASSERT(event != NULL);
}
