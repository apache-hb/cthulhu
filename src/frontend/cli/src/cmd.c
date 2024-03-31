// SPDX-License-Identifier: GPL-3.0-only

#include "cmd.h"

#include "config/config.h"

#include "cthulhu/broker/broker.h"
#include "format/notify.h"
#include "std/vector.h"
#include "std/str.h"

#include "notify/notify.h"

#include "argparse/argparse.h"

static const cfg_info_t kConfigInfo = {
    .name = "cli",
    .brief = "Cthulhu CLI configuration options",
};

static const cfg_info_t kReportInfo = {
    .name = "reports",
    .brief = "Reporting options"
};

static const char *const kLangShortArgs[] = CT_ARGS("l");
static const char *const kLangLongArgs[] = CT_ARGS("lang");

static const cfg_info_t kLang = {
    .name = "lang",
    .brief = "Load a language driver",
    .short_args = kLangShortArgs,
    .long_args = kLangLongArgs,
};

static const char *const kPluginShortArgs[] = CT_ARGS("p");
static const char *const kPluginLongArgs[] = CT_ARGS("plugin");

static const cfg_info_t kPlugin = {
    .name = "plugin",
    .brief = "Load a plugin",
    .short_args = kPluginShortArgs,
    .long_args = kPluginLongArgs,
};

static const char *const kTargetShortArgs[] = CT_ARGS("t");
static const char *const kTargetLongArgs[] = CT_ARGS("target");

static const cfg_info_t kTarget = {
    .name = "target",
    .brief = "Load a target",
    .short_args = kTargetShortArgs,
    .long_args = kTargetLongArgs,
};

static const char *const kIrShortArgs[] = CT_ARGS("ir");
static const char *const kIrLongArgs[] = CT_ARGS("emit-ssa");

static const cfg_info_t kEmitIr = {
    .name = "emit-ssa",
    .brief = "Emit SSA IR to the output directory",
    .short_args = kIrShortArgs,
    .long_args = kIrLongArgs,
};

static const char *const kTreeShortArgs[] = CT_ARGS("tree");
static const char *const kTreeLongArgs[] = CT_ARGS("emit-tree");

static const cfg_info_t kEmitTree = {
    .name = "emit-tree",
    .brief = "Emit the parse tree to the output directory",
    .short_args = kTreeShortArgs,
    .long_args = kTreeLongArgs,
};

static const char *const kFileLayoutArgs[] = CT_ARGS("file-layout");

static const cfg_info_t kFileLayout = {
    .name = "file-layout",
    .brief = "File layout to use",
    .short_args = kFileLayoutArgs,
};

static const cfg_choice_t kFileLayoutChoices[] = {
    { "pair", eFileLayoutPair },
    { "single", eFileLayoutSingle },
    { "tree", eFileLayoutTree },
    { "flat", eFileLayoutFlat },
};

static const char *const kTargetOutputArgs[] = CT_ARGS("target-output");

static const cfg_info_t kTargetOutput = {
    .name = "target-output",
    .brief = "Target to use for output generation",
    .short_args = kTargetOutputArgs,
};

static const char *const kWarnAsErrorShortArgs[] = CT_ARGS("Werror", "WX");

static const cfg_info_t kWarnAsError = {
    .name = "warn-as-error",
    .brief = "Treat warnings as errors",
    .short_args = kWarnAsErrorShortArgs
};

static const char *const kReportLimitShortArgs[] = CT_ARGS("fmax-errors");

static const cfg_info_t kReportLimit = {
    .name = "max-errors",
    .brief = "Limit the number of reports",
    .short_args = kReportLimitShortArgs
};

static const char *const kOutputDirShortArgs[] = CT_ARGS("o");
static const char *const kOutputDirLongArgs[] = CT_ARGS("output-dir");

static const cfg_info_t kOutputDir = {
    .name = "output-dir",
    .brief = "Output directory for generated files",
    .short_args = kOutputDirShortArgs,
    .long_args = kOutputDirLongArgs,
};

static const char *const kReportStyleShortArgs[] = CT_ARGS("report");
static const char *const kReportStyleLongArgs[] = CT_ARGS("report-style");

static const cfg_info_t kReportStyle = {
    .name = "report-style",
    .brief = "Report style to use",
    .short_args = kReportStyleShortArgs,
    .long_args = kReportStyleLongArgs,
};

static const cfg_choice_t kReportStyleChoices[] = {
    { "simple", eTextSimple },
    { "complex", eTextComplex },
};

tool_t make_tool(arena_t *arena)
{
    cfg_group_t *config = config_root(&kConfigInfo, arena);

    default_options_t options = get_default_options(config);

    cfg_field_t *add_language_field = config_vector(config, &kLang, NULL);

    cfg_field_t *add_plugin_field = config_vector(config, &kPlugin, NULL);
    cfg_field_t *add_target_field = config_vector(config, &kTarget, NULL);

    cfg_field_t *emit_tree_field = config_bool(config, &kEmitTree, false);
    cfg_field_t *emit_ir_field = config_bool(config, &kEmitIr, false);

    cfg_field_t *output_dir_field = config_string(config, &kOutputDir, NULL);

    cfg_enum_t file_layout_options = {
        .options = kFileLayoutChoices,
        .count = (sizeof(kFileLayoutChoices) / sizeof(cfg_choice_t)),
        .initial = eFileLayoutPair,
    };
    cfg_field_t *file_layout_field = config_enum(config, &kFileLayout, file_layout_options);

    cfg_field_t *output_target_field = config_string(config, &kTargetOutput, NULL);

    cfg_group_t *report_group = config_group(config, &kReportInfo);
    cfg_field_t *warn_as_error_field = config_bool(report_group, &kWarnAsError, false);

    cfg_int_t report_limit_options = {.initial = 20, .min = 0, .max = 1000};
    cfg_field_t *report_limit_field = config_int(report_group, &kReportLimit, report_limit_options);

    cfg_enum_t report_style_options = {
        .options = kReportStyleChoices,
        .count = (sizeof(kReportStyleChoices) / sizeof(cfg_choice_t)),
        .initial = eTextSimple,
    };
    cfg_field_t *report_style_field = config_enum(report_group, &kReportStyle, report_style_options);

    tool_t tool = {
        .config = config,
        .options = options,

        .add_language = add_language_field,
        .add_plugin = add_plugin_field,
        .add_target = add_target_field,

        .emit_tree = emit_tree_field,
        .emit_ssa = emit_ir_field,
        .output_dir = output_dir_field,
        .output_layout = file_layout_field,
        .output_target = output_target_field,

        .warn_as_error = warn_as_error_field,
        .report_limit = report_limit_field,
        .report_style = report_style_field,
    };

    return tool;
}
