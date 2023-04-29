#include "report/report.h"
#include "argparse2/argparse.h"
#include "cthulhu/mediator/mediator.h"

#include <stdio.h>

typedef struct runtime_t 
{
    reports_t *reports;
    ap_t *ap;
    mediator_t *mediator;
} runtime_t;

// general
static const char *kLoadLangNames[] = { "-lang", "--load-lang", NULL };
static const char *kLoadPluginNames[] = { "-plugin", "--load-plugin", NULL };

// codegen
static const char *kOutputFileNames[] = { "-o", "--output", NULL };
static const char *kOutputGenNames[] = { "-cg", "--codegen", NULL };
static const char *kOutputHeaderNames[] = { "-h", "--header", NULL };

// debug
static const char *kDebugSsaNames[] = { "-dbgssa", "--debug-ssa", NULL };

static ap_event_result_t on_load_language(ap_t *ap, const ap_param_t *param, const void *value, void *data)
{
    UNUSED(ap);
    UNUSED(param);
    UNUSED(data);

    const char *path = value;
    // runtime_t *rt = data;

    printf("loading language from `%s`\n", path);

    return eEventHandled;
}

static ap_event_result_t on_load_plugin(ap_t *ap, const ap_param_t *param, const void *value, void *data)
{
    UNUSED(ap);
    UNUSED(param);
    UNUSED(data);

    const char *path = value;
    // runtime_t *rt = data;

    printf("loading plugin from `%s`\n", path);

    return eEventHandled;
}

int main(int argc, const char **argv)
{
    runtime_init();
    reports_t *reports = begin_reports();
    mediator_t *mediator = mediator_new("cli", NEW_VERSION(0, 0, 1));
    ap_t *ap = ap_new("cli compiler", NEW_VERSION(0, 0, 1));

    runtime_t rt = {
        .reports = reports,
        .ap = ap,
        .mediator = mediator
    };

    ap_group_t *general = ap_group_new(ap, "general", "general options");
    ap_param_t *loadLanguageParam = ap_add_string(general, "load language", kLoadLangNames);
    ap_param_t *loadPluginParam = ap_add_string(general, "load plugin", kLoadPluginNames);

    ap_group_t *codegen = ap_group_new(ap, "codegen", "code generation options");
    ap_param_t *outputFileParam = ap_add_string(codegen, "output file name (default: out)", kOutputFileNames);
    ap_param_t *outputGenParam = ap_add_string(codegen, "output generator [ssa-c89, hlir-c89] (default: ssa-c89)", kOutputGenNames);
    ap_param_t *outputHeaderParam = ap_add_string(codegen, "output header (default: none)", kOutputHeaderNames);

    ap_group_t *debug = ap_group_new(ap, "debug", "debug options");
    ap_param_t *debugSsaParam = ap_add_bool(debug, "debug ssa", kDebugSsaNames);

    ap_event(ap, loadLanguageParam, on_load_language, &rt);
    ap_event(ap, loadPluginParam, on_load_plugin, &rt);

    ap_parse(ap, reports, argc, argv);

    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; ++i)
    {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
}
