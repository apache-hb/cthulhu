// SPDX-License-Identifier: GPL-3.0-only

#include "base/panic.h"
#include "config/config.h"
#include "core/macros.h"
#include "cthulhu/broker/broker.h"
#include "cthulhu/events/events.h"
#include "setup/setup.h"
#include "io/console.h"
#include "io/io.h"
#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include "support/loader.h"
#include "argparse/argparse.h"
#include "arena/arena.h"

#include "memory/memory.h"
#include "support/support.h"

static const frontend_t kFrontendInfo = {
    .info = {
        .id = "frontend-diag",
        .name = "Diagnostic query tool",
        .version = {
            .license = "GPLv3",
            .author = "Elliot Haisley",
            .desc = "Cthulhu diagnostic lookup and query tool",
            .version = CT_NEW_VERSION(0, 0, 1),
        },
    },
};

static const cfg_info_t kGroupInfo = {
    .name = "diagnostics",
    .brief = "Diagnostic query options"
};

static const cfg_arg_t kPrintLangsArgs[] = { ARG_LONG("all-langs") };

static const cfg_info_t kPrintLangsInfo = {
    .name = "langs",
    .brief = "Print info about all available languages",
    .args = CT_ARGS(kPrintLangsArgs),
};

static const cfg_arg_t kPrintSingleLangArgs[] = { ARG_LONG("single-lang"), ARG_LONG("lang") };

static const cfg_info_t kPrintSingleLangInfo = {
    .name = "lang",
    .brief = "Print information about a specific language",
    .args = CT_ARGS(kPrintSingleLangArgs),
};

static const cfg_arg_t kPrintDiagInfoArgs[] = { ARG_LONG("all") };

static const cfg_info_t kPrintDiagsInfo = {
    .name = "diags",
    .brief = "Print all available diagnostics for all languages",
    .args = CT_ARGS(kPrintDiagInfoArgs),
};

static const cfg_arg_t kPrintSingleDiagInfoArgs[] = { ARG_LONG("diag") };

static const cfg_info_t kPrintSingleDiagInfo = {
    .name = "lang-diag",
    .brief = "Print all available diagnostics for a specific language",
    .args = CT_ARGS(kPrintSingleDiagInfoArgs)
};

typedef struct tool_t
{
    cfg_group_t *root;

    cfg_field_t *print_all_langs;
    cfg_field_t *print_one_lang;

    cfg_field_t *print_all_diags;
    cfg_field_t *print_one_diag;

    setup_options_t options;
} tool_t;

static tool_t make_config(typevec_t *langs, arena_t *arena)
{
    cfg_group_t *root = config_root(&kGroupInfo, arena);
    size_t len = typevec_len(langs);

    cfg_choice_t *lang_choices = ARENA_MALLOC(sizeof(cfg_choice_t) * (len + 1), "lang_choices", root, arena);
    cfg_choice_t none_choice = {
        .text = "none",
        .value = 0
    };

    lang_choices[0] = none_choice;

    for (size_t i = 0; i < len; i++)
    {
        const language_t *lang = typevec_offset(langs, i);
        module_info_t info = lang->info;
        cfg_choice_t choice = {
            .text = info.id,
            .value = i + 1
        };

        lang_choices[i + 1] = choice;
    }

    cfg_enum_t lang_options = {
        .options = lang_choices,
        .count = len + 1,
        .initial = 0
    };

    setup_options_t options = setup_options(kFrontendInfo.info.version, root);

    cfg_field_t *print_all_langs = config_bool(root, &kPrintLangsInfo, false);

    cfg_field_t *print_one_lang = config_enum(root, &kPrintSingleLangInfo, lang_options);

    cfg_field_t *print_all_diags = config_bool(root, &kPrintDiagsInfo, false);

    cfg_field_t *print_one_diag = config_enum(root, &kPrintSingleDiagInfo, lang_options);

    tool_t config = {
        .root = root,
        .print_all_langs = print_all_langs,
        .print_one_lang = print_one_lang,

        .print_all_diags = print_all_diags,
        .print_one_diag = print_one_diag,

        .options = options,
    };

    return config;
}

typedef struct diag_search_t
{
    map_t *ids;

    typevec_t *diagnostics;
} diag_search_t;

static void add_diagnostic(diag_search_t *ctx, const diagnostic_t *diag)
{
    const diagnostic_t *old = map_get(ctx->ids, diag->id);
    CTASSERTF(old == NULL, "duplicate diagnostic id: %s (old message %s)", diag->id, old->brief);

    map_set(ctx->ids, diag->id, (diagnostic_t*)diag);
    typevec_push(ctx->diagnostics, diag);
}

static void add_diagnostics(diag_search_t *ctx, diagnostic_list_t diagnostics)
{
    for (size_t i = 0; i < diagnostics.count; i++)
    {
        add_diagnostic(ctx, diagnostics.diagnostics[i]);
    }
}

static size_t count_diagnostics(const typevec_t *langs)
{
    size_t count = 0;

    size_t len = typevec_len(langs);
    for (size_t i = 0; i < len; i++)
    {
        const language_t *lang = typevec_offset(langs, i);
        module_info_t info = lang->info;
        count += info.diagnostics.count;
    }

    return count;
}

// TODO: this should all be part of the support library

static void print_lang_info(io_t *io, const language_t *lang)
{
    module_info_t info = lang->info;
    diagnostic_list_t diagnostics = info.diagnostics;
    version_info_t version = info.version;

    int major = CT_VERSION_MAJOR(version.version);
    int minor = CT_VERSION_MINOR(version.version);
    int patch = CT_VERSION_PATCH(version.version);

    io_printf(io, "%s:\n", info.name);
    io_printf(io, "  id: %s\n", info.id);
    io_printf(io, "  desc: %s\n", version.desc);
    io_printf(io, "  author: %s\n", version.author);
    io_printf(io, "  license: %s\n", version.license);
    io_printf(io, "  version: %d.%d.%d\n", major, minor, patch);

    io_printf(io, "  %zu diagnostics\n", diagnostics.count);
}

static void print_all_langs(io_t *io, typevec_t *langs)
{
    size_t len = typevec_len(langs);
    io_printf(io, "%zu available languages:\n", len);
    for (size_t i = 0; i < len; i++)
    {
        const language_t *lang = typevec_offset(langs, i);
        print_lang_info(io, lang);
    }
}

static void print_diagnostic(io_t *io, const diagnostic_t *diag)
{
    io_printf(io, "%s: %s\n", diag->id, severity_string(diag->severity));
    io_printf(io, "brief: %s\n", diag->brief);
    io_printf(io, "description: %s\n", diag->description);
}

int main(int argc, const char **argv)
{
    setup_default(NULL);
    arena_t *arena = get_global_arena();
    io_t *io = io_stdout();

    broker_t *broker = broker_new(&kFrontendInfo, arena);
    loader_t *loader = loader_new(arena);
    support_t *support = support_new(broker, loader, arena);
    support_load_default_modules(support);

    typevec_t *mods = support_get_modules(support);

    size_t len = typevec_len(mods);
    typevec_t *langs = typevec_new(sizeof(language_t), len, arena);
    for (size_t i = 0; i < len; i++)
    {
        const loaded_module_t *mod = typevec_offset(mods, i);
        if (mod->type & eModLanguage)
        {
            typevec_push(langs, mod->lang);
        }
    }

    tool_t tool = make_config(langs, arena);

    setup_init_t init = setup_parse(argc, argv, tool.options);

    if (setup_should_exit(&init))
        return setup_exit_code(&init);

    ///
    /// print all langs
    ///

    if (cfg_bool_value(tool.print_all_langs))
    {
        print_all_langs(io, langs);
        return CT_EXIT_OK;
    }

    ///
    /// print one lang
    ///

    size_t lang_index = cfg_enum_value(tool.print_one_lang);
    if (lang_index != 0)
    {
        const language_t *lang = typevec_offset(langs, lang_index - 1);
        print_lang_info(io, lang);
        return CT_EXIT_OK;
    }

    ///
    /// collect all diagnostics
    ///

    diagnostic_list_t common = get_common_diagnostics();

    size_t count = count_diagnostics(langs) + common.count;

    diag_search_t ctx = {
        .ids = map_optimal(count, kTypeInfoString, arena),
        .diagnostics = typevec_new(sizeof(diagnostic_t), count, arena),
    };

    add_diagnostics(&ctx, common);

    size_t lang_count = typevec_len(langs);
    for (size_t i = 0; i < lang_count; i++)
    {
        const language_t *lang = typevec_offset(langs, i);
        module_info_t info = lang->info;
        add_diagnostics(&ctx, info.diagnostics);
    }

    ///
    /// print all diags
    ///

    if (cfg_bool_value(tool.print_all_diags))
    {
        size_t diag_count = typevec_len(ctx.diagnostics);
        io_printf(io, "%zu diagnostics:\n", diag_count);
        for (size_t i = 0; i < diag_count; i++)
        {
            const diagnostic_t *diag = typevec_offset(ctx.diagnostics, i);
            print_diagnostic(io, diag);

            if (i + 1 < diag_count)
            {
                io_printf(io, "\n");
            }
        }

        return CT_EXIT_OK;
    }

    ///
    /// print all diags for a specific lang
    ///

    size_t lang_diag_index = cfg_enum_value(tool.print_one_diag);
    if (lang_diag_index != 0)
    {
        const language_t *lang = typevec_offset(langs, lang_diag_index - 1);

        module_info_t info = lang->info;
        size_t diag_count = info.diagnostics.count;
        io_printf(io, "%zu diagnostics for %s:\n", diag_count, info.name);

        for (size_t i = 0; i < diag_count; i++)
        {
            const diagnostic_t *diag = info.diagnostics.diagnostics[i];
            print_diagnostic(io, diag);

            if (i + 1 < diag_count)
            {
                io_printf(io, "\n");
            }
        }
    }

    ///
    /// print info about specific diagnostics
    ///

    vector_t *posargs = init.posargs;
    size_t posarg_count = vector_len(posargs);
    for (size_t i = 0; i < posarg_count; i++)
    {
        const char *arg = vector_get(posargs, i);
        const diagnostic_t *diag = map_get(ctx.ids, arg);
        if (diag == NULL)
        {
            io_printf(io, "unknown diagnostic: %s\n", arg);
        }
        else
        {
            print_diagnostic(io, diag);

            if (i + 1 < posarg_count)
            {
                io_printf(io, "\n");
            }
        }
    }

    io_close(io);
}
