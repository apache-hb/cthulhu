#include "cthulhu/mediator/mediator.h"

#include <windows.h>

#include <stdio.h>
#include <winnt.h>

static const char *kRegionNames[] = {
    [eRegionLoad] = "Load",
    [eRegionInit] = "Init",
    [eRegionLoadSource] = "Load source",
    [eRegionParse] = "Parse",
    [eRegionCompile] = "Compile",
    [eRegionOptimize] = "Optimize",
    [eRegionCodegen] = "Codegen",
    [eRegionCleanup] = "Cleanup",
    [eRegionEnd] = "End"
};

typedef struct timing_t
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;

    LONGLONG ticks[eRegionTotal];
} timing_t;

static void plugin_init(plugin_handle_t *handle)
{
    timing_t timing = { 0 };
    QueryPerformanceFrequency(&timing.frequency);
    QueryPerformanceCounter(&timing.counter);
}

static void plugin_region(plugin_handle_t *handle, region_t region)
{
    timing_t *timing = handle->user;
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    LONGLONG ticks = counter.QuadPart - timing->counter.QuadPart;
    timing->ticks[region] += ticks;

    timing->counter = counter;
}

static void plugin_shutdown(plugin_handle_t *handle)
{
    timing_t *timing = handle->user;

    double freq = (double)timing->frequency.QuadPart;
    printf("Timing:\n");

    for (size_t i = 0; i < eRegionTotal; i++)
    {
        printf("- %s: %fms\n", kRegionNames[i], (double)timing->ticks[i] / freq * 1000.0);
    }
}

static const plugin_t kPluginInfo = {
    .name = "Compiler execution timing",
    .version = NEW_VERSION(1, 0, 0),
    .fnInit = plugin_init,
    .fnShutdown = plugin_shutdown,
    .fnRegion = plugin_region,
};

PLUGIN_EXPORT
extern const plugin_t *PLUGIN_ENTRY_POINT(mediator_t *mediator)
{
    return &kPluginInfo;
}
