#include "base/panic.h"
#include "config/config.h"
#include "core/macros.h"
#include "cthulhu/runtime/interface.h"
#include "cthulhu/events/events.h"
#include "defaults/defaults.h"
#include "io/console.h"
#include "io/io.h"
#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include "support/langs.h"
#include "argparse/argparse.h"

#include "memory/memory.h"

static const version_info_t kToolVersion = {
    .license = "GPLv3",
    .author = "Elliot Haisley",
    .desc = "Cthulhu diagnostic lookup and query tool",
    .version = NEW_VERSION(0, 0, 1),
};

static const cfg_info_t kGroupInfo = {
    .name = "diagnostics",
    .brief = "Diagnostic query options"
};

static const char *const kPrintLangsArgs[] = { "langs", NULL };

static const cfg_info_t kPrintLangsInfo = {
    .name = "langs",
    .brief = "Print info about all available languages",
    .short_args = kPrintLangsArgs
};

static const char *const kPrintSingleLangArgs[] = { "lang", NULL };

static const cfg_info_t kPrintSingleLangInfo = {
    .name = "lang",
    .brief = "Print information about a specific language",
    .short_args = kPrintSingleLangArgs
};

static const char *const kPrintDiagInfoArgs[] = { "all-diags", NULL };

static const cfg_info_t kPrintDiagsInfo = {
    .name = "diags",
    .brief = "Print all available diagnostics for all languages",
    .short_args = kPrintDiagInfoArgs
};

static const char *const kPrintSingleDiagInfoArgs[] = { "lang-diag", NULL };

static const cfg_info_t kPrintSingleDiagInfo = {
    .name = "lang-diag",
    .brief = "Print all available diagnostics for a specific language",
    .short_args = kPrintSingleDiagInfoArgs
};

typedef struct tool_t
{
    cfg_group_t *root;

    cfg_field_t *print_all_langs;
    cfg_field_t *print_one_lang;

    cfg_field_t *print_all_diags;
    cfg_field_t *print_one_diag;

    default_options_t options;
} tool_t;

static tool_t make_config(arena_t *arena, langs_t langs)
{
    cfg_group_t *root = config_root(arena, &kGroupInfo);

    cfg_choice_t *lang_choices = ARENA_MALLOC(arena, sizeof(cfg_choice_t) * (langs.size + 1), "lang_choices", root);
    cfg_choice_t none_choice = {
        .text = "none",
        .value = 0
    };

    lang_choices[0] = none_choice;

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs[i];
        cfg_choice_t choice = {
            .text = lang->id,
            .value = i + 1
        };

        lang_choices[i + 1] = choice;
    }

    cfg_enum_t lang_options = {
        .options = lang_choices,
        .count = langs.size + 1,
        .initial = 0
    };

    default_options_t options = get_default_options(root);

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
    CTASSERTF(old == NULL, "duplicate diagnostic id: %s", diag->id);

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

static size_t count_diagnostics(const langs_t *langs)
{
    size_t count = 0;

    for (size_t i = 0; i < langs->size; i++)
    {
        const language_t *lang = langs->langs[i];
        count += lang->diagnostics.count;
    }

    return count;
}

// TODO: this should all be part of the support library

static void print_lang_info(io_t *io, const language_t *lang)
{
    diagnostic_list_t diagnostics = lang->diagnostics;
    version_info_t version = lang->version;

    int major = VERSION_MAJOR(version.version);
    int minor = VERSION_MINOR(version.version);
    int patch = VERSION_PATCH(version.version);

    io_printf(io, "%s:\n", lang->name);
    io_printf(io, "  id: %s\n", lang->id);
    io_printf(io, "  desc: %s\n", version.desc);
    io_printf(io, "  author: %s\n", version.author);
    io_printf(io, "  license: %s\n", version.license);
    io_printf(io, "  version: %d.%d.%d\n", major, minor, patch);

    io_printf(io, "  %zu diagnostics\n", diagnostics.count);
}

static void print_all_langs(io_t *io, langs_t langs)
{
    io_printf(io, "%zu available languages:\n", langs.size);
    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs[i];
        print_lang_info(io, lang);
    }
}

static const char *severity_to_string(severity_t severity)
{
    // TODO: this should be part of notify
    switch (severity)
    {
    case eSeveritySorry:
        return "sorry";
    case eSeverityInternal:
        return "ice";
    case eSeverityFatal:
        return "fatal";
    case eSeverityWarn:
        return "warn";
    case eSeverityInfo:
        return "info";
    case eSeverityDebug:
        return "debug";
    default:
        NEVER("unknown severity %d", severity);
    }
}

static void print_diagnostic(io_t *io, const diagnostic_t *diag)
{
    io_printf(io, "%s: %s\n", diag->id, severity_to_string(diag->severity));
    io_printf(io, "brief: %s\n", diag->brief);
    io_printf(io, "description: %s\n", diag->description);
}

int main(int argc, const char **argv)
{
    default_init();
    arena_t *arena = get_global_arena();
    io_t *io = io_stdout();

    langs_t langs = get_langs();
    tool_t tool = make_config(arena, langs);

    tool_config_t config = {
        .arena = arena,
        .io = io,

        .group = tool.root,
        .version = kToolVersion,

        .argc = argc,
        .argv = argv,
    };

    ap_t *ap = ap_new(config.group, config.arena);

    int err = parse_argparse(ap, tool.options, config);
    if (err == EXIT_SHOULD_EXIT)
    {
        return EXIT_OK;
    }

    ///
    /// print all langs
    ///

    if (cfg_bool_value(tool.print_all_langs))
    {
        print_all_langs(io, langs);
        return EXIT_OK;
    }

    ///
    /// print one lang
    ///

    size_t lang_index = cfg_enum_value(tool.print_one_lang);
    if (lang_index != 0)
    {
        const language_t *lang = langs.langs[lang_index - 1];
        print_lang_info(io, lang);
        return EXIT_OK;
    }

    ///
    /// collect all diagnostics
    ///

    diagnostic_list_t common = get_common_diagnostics();

    size_t count = count_diagnostics(&langs) + common.count;

    diag_search_t ctx = {
        .ids = map_optimal(count, arena),
        .diagnostics = typevec_new(sizeof(diagnostic_t), count, arena),
    };

    add_diagnostics(&ctx, common);

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs[i];
        add_diagnostics(&ctx, lang->diagnostics);
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

        return EXIT_OK;
    }

    ///
    /// print all diags for a specific lang
    ///

    size_t lang_diag_index = cfg_enum_value(tool.print_one_diag);
    if (lang_diag_index != 0)
    {
        const language_t *lang = langs.langs[lang_diag_index - 1];

        size_t diag_count = lang->diagnostics.count;
        io_printf(io, "%zu diagnostics for %s:\n", diag_count, lang->name);

        for (size_t i = 0; i < diag_count; i++)
        {
            const diagnostic_t *diag = lang->diagnostics.diagnostics[i];
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

    vector_t *posargs = ap_get_posargs(ap);
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
