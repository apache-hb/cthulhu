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

static const cfg_arg_t kLangArgs[] = { ARG_SHORT("l"), ARG_LONG("lang") };

static const cfg_info_t kLang = {
    .name = "lang",
    .brief = "Load a language driver",
    .args = CT_ARGS(kLangArgs),
};

static const cfg_arg_t kPluginArgs[] = { ARG_SHORT("p"), ARG_LONG("plugin") };

static const cfg_info_t kPlugin = {
    .name = "plugin",
    .brief = "Load a plugin",
    .args = CT_ARGS(kPluginArgs),
};

static const cfg_arg_t kTargetArgs[] = { ARG_SHORT("t"), ARG_LONG("target") };

static const cfg_info_t kTarget = {
    .name = "target",
    .brief = "Load a target",
    .args = CT_ARGS(kTargetArgs),
};

static const cfg_arg_t kIrArgs[] = { ARG_SHORT("ir"), ARG_LONG("emit-ssa") };

static const cfg_info_t kEmitIr = {
    .name = "emit-ssa",
    .brief = "Emit SSA IR to the output directory",
    .args = CT_ARGS(kIrArgs),
};

static const cfg_arg_t kTreeArgs[] = { ARG_SHORT("tree"), ARG_LONG("emit-tree") };

static const cfg_info_t kEmitTree = {
    .name = "emit-tree",
    .brief = "Emit the parse tree to the output directory",
    .args = CT_ARGS(kTreeArgs),
};

static const cfg_arg_t kFileLayoutArgs[] = { ARG_LONG("file-layout") };

static const cfg_info_t kFileLayout = {
    .name = "file-layout",
    .brief = "File layout to use",
    .args = CT_ARGS(kFileLayoutArgs),
};

static const cfg_choice_t kFileLayoutChoices[] = {
    { "pair", eFileLayoutPair },
    { "single", eFileLayoutSingle },
    { "tree", eFileLayoutTree },
    { "flat", eFileLayoutFlat },
};

static const cfg_arg_t kTargetOutputArgs[] = { ARG_LONG("target-output") };

static const cfg_info_t kTargetOutput = {
    .name = "target-output",
    .brief = "Target to use for output generation",
    .args = CT_ARGS(kTargetOutputArgs),
};

static const cfg_arg_t kWarnAsErrorArgs[] = { ARG_SHORT("Werror"), ARG_SHORT("WX") };

static const cfg_info_t kWarnAsError = {
    .name = "warn-as-error",
    .brief = "Treat warnings as errors",
    .args = CT_ARGS(kWarnAsErrorArgs),
};

static const cfg_arg_t kReportLimitArgs[] = { ARG_SHORT("ferror-limit"), ARG_SHORT("fmax-errors") };

static const cfg_info_t kReportLimit = {
    .name = "max-errors",
    .brief = "Limit the number of reports",
    .args = CT_ARGS(kReportLimitArgs),
};

static const cfg_arg_t kOutputDirArgs[] = { ARG_SHORT("o"), ARG_LONG("output-dir") };

static const cfg_info_t kOutputDir = {
    .name = "output-dir",
    .brief = "Output directory for generated files",
    .args = CT_ARGS(kOutputDirArgs),
};

static const cfg_arg_t kReportStyleArgs[] = { ARG_SHORT("report"), ARG_LONG("report-style") };

static const cfg_info_t kReportStyle = {
    .name = "report-style",
    .brief = "Report style to use",
    .args = CT_ARGS(kReportStyleArgs),
};

static const cfg_choice_t kReportStyleChoices[] = {
    { "simple", eTextSimple },
    { "complex", eTextComplex },
};

tool_t make_tool(version_info_t version, arena_t *arena)
{
    cfg_group_t *config = config_root(&kConfigInfo, arena);

    setup_options_t options = setup_options(version, config);

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

    cfg_field_t *warn_as_error_field = config_bool(options.report.group, &kWarnAsError, false);

    cfg_int_t report_limit_options = {.initial = 20, .min = 0, .max = 1000};
    cfg_field_t *report_limit_field = config_int(options.report.group, &kReportLimit, report_limit_options);

    cfg_enum_t report_style_options = {
        .options = kReportStyleChoices,
        .count = (sizeof(kReportStyleChoices) / sizeof(cfg_choice_t)),
        .initial = eTextSimple,
    };
    cfg_field_t *report_style_field = config_enum(options.report.group, &kReportStyle, report_style_options);

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
