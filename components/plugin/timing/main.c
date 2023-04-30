#include "cthulhu/mediator/mediator.h"

#include <windows.h>

#include <stdio.h>

static LARGE_INTEGER gFrequency;
static LARGE_INTEGER gCounter;

static LONGLONG gTicks[eRegionTotal] = { 0 };

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

static void plugin_init(mediator_t *mediator)
{
    QueryPerformanceFrequency(&gFrequency);
    QueryPerformanceCounter(&gCounter);
}

static void plugin_region(mediator_t *mediator, region_t region)
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    LONGLONG ticks = counter.QuadPart - gCounter.QuadPart;
    gTicks[region] += ticks;

    gCounter = counter;
}

static void plugin_shutdown(mediator_t *mediator)
{
    printf("Timing:\n");

    for (size_t i = 0; i < eRegionTotal; i++)
    {
        printf("- %s: %fms\n", kRegionNames[i], (double)gTicks[i] / (double)gFrequency.QuadPart * 1000.0);
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
