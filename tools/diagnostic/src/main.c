#include "argparse/argparse.h"
#include "base/panic.h"
#include "config/config.h"
#include "cthulhu/mediator/interface.h"
#include "cthulhu/events/events.h"
#include "io/console.h"
#include "io/io.h"
#include "std/map.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include "support/langs.h"

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

static const char *const kPrintHelpShortArgs[] = { "h", "?", NULL };
static const char *const kPrintHelpLongArgs[] = { "help", NULL };

static const cfg_info_t kPrintHelpInfo = {
    .name = "help",
    .brief = "Print help information",
    .short_args = kPrintHelpShortArgs,
    .long_args = kPrintHelpLongArgs
};

static const char *const kPrintVersionShortArgs[] = { "V", NULL };
static const char *const kPrintVersionLongArgs[] = { "version", NULL };

static const cfg_info_t kPrintVersionInfo = {
    .name = "version",
    .brief = "Print version information",
    .short_args = kPrintVersionShortArgs,
    .long_args = kPrintVersionLongArgs
};

static const char *const kPrintLangsArgs[] = { "langs", NULL };

static const cfg_info_t kPrintLangsInfo = {
    .name = "langs",
    .brief = "Print available languages",
    .short_args = kPrintLangsArgs,
    .long_args = kPrintLangsArgs,
};

static const char *const kPrintDiagInfoArgs[] = { "all", NULL };

static const cfg_info_t kPrintDiagsInfo = {
    .name = "diags",
    .brief = "Print all available diagnostics for all languages",
    .short_args = kPrintDiagInfoArgs,
    .long_args = kPrintDiagInfoArgs,
};

typedef struct diag_config_t
{
    config_t *root;

    cfg_field_t *help;
    cfg_field_t *version;
    cfg_field_t *langs;
    cfg_field_t *diags;
} diag_config_t;

static diag_config_t make_config(arena_t *arena)
{
    config_t *root = config_new(arena, &kGroupInfo);
    cfg_bool_t help_options = { .initial = false };
    cfg_field_t *help = config_bool(root, &kPrintHelpInfo, help_options);

    cfg_bool_t version_options = { .initial = false };
    cfg_field_t *version = config_bool(root, &kPrintVersionInfo, version_options);

    cfg_bool_t langs_options = { .initial = false };
    cfg_field_t *langs = config_bool(root, &kPrintLangsInfo, langs_options);

    cfg_bool_t diags_options = { .initial = false };
    cfg_field_t *diags = config_bool(root, &kPrintDiagsInfo, diags_options);

    diag_config_t config = {
        .root = root,
        .help = help,
        .version = version,
        .langs = langs,
        .diags = diags
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
        const language_t *lang = langs->langs + i;
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
        const language_t *lang = langs.langs + i;
        print_lang_info(io, lang);
    }
}

static void print_config_field(io_t *io, const cfg_field_t *field)
{
    const cfg_info_t *info = cfg_get_info(field);
    const char *name = info->name;
    const char *brief = info->brief;

    io_printf(io, "  ");

    if (info->short_args)
    {
        for (size_t i = 0; info->short_args[i]; i++)
        {
            io_printf(io, "-%s", info->short_args[i]);
            if (info->short_args[i + 1])
            {
                io_printf(io, ", ");
            }
        }
    }

    if (info->long_args)
    {
        if (info->short_args)
        {
            io_printf(io, ", ");
        }

        for (size_t i = 0; info->long_args[i]; i++)
        {
            io_printf(io, "--%s", info->long_args[i]);
            if (info->long_args[i + 1])
            {
                io_printf(io, ", ");
            }
        }
    }

    io_printf(io, " ");

    switch (cfg_get_type(field))
    {
    case eConfigBool:
        break;
    case eConfigInt:
        io_printf(io, "=[int]");
        break;
    case eConfigString:
        io_printf(io, "=[string]");
        break;
    case eConfigEnum:
        io_printf(io, "=[enum]");
        break;
    case eConfigFlags:
        io_printf(io, "=[flags]");
        break;
    default:
        NEVER("unknown config type %d", cfg_get_type(field));
    }

    io_printf(io, "  %s: %s\n", name, brief);
}

static void print_config_group(io_t *io, const config_t *group)
{
    const cfg_info_t *info = cfg_group_info(group);
    io_printf(io, "%s:\n", info->name);

    vector_t *fields = cfg_get_fields(group);
    size_t field_count = vector_len(fields);

    for (size_t i = 0; i < field_count; i++)
    {
        const cfg_field_t *field = vector_get(fields, i);
        print_config_field(io, field);
    }

    typevec_t *groups = cfg_get_groups(group);
    size_t group_count = typevec_len(groups);

    for (size_t i = 0; i < group_count; i++)
    {
        const config_t *child = typevec_offset(groups, i);
        print_config_group(io, child);
    }
}

static void print_help(io_t *io, const char *name, diag_config_t config)
{
    io_printf(io, "usage: %s [options] [diagnostics...]\n", name);
    io_printf(io, "\n");
    print_config_group(io, config.root);
}

static void print_version(io_t *io)
{
    int major = VERSION_MAJOR(kToolVersion.version);
    int minor = VERSION_MINOR(kToolVersion.version);
    int patch = VERSION_PATCH(kToolVersion.version);

    io_printf(io, "%s\n", kToolVersion.desc);
    io_printf(io, "version: %d.%d.%d\n", major, minor, patch);
    io_printf(io, "author: %s\n", kToolVersion.author);
    io_printf(io, "license: %s\n", kToolVersion.license);
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
    arena_t *arena = ctu_default_alloc();
    runtime_init(arena);

    diag_config_t root = make_config(arena);

    io_t *io = io_stdout(arena);
    ap_t *ap = ap_new(root.root, arena);
    ap_parse(ap, argc, argv);

    vector_t *unknown = ap_get_unknown(ap);
    size_t unknown_count = vector_len(unknown);
    for (size_t i = 0; i < unknown_count; i++)
    {
        const char *arg = vector_get(unknown, i);
        io_printf(io, "unknown argument: %s\n", arg);
    }

    if (cfg_bool_value(root.help))
    {
        print_help(io, argv[0], root);
        goto finish;
    }

    if (cfg_bool_value(root.version))
    {
        print_version(io);
        goto finish;
    }

    langs_t langs = get_langs();

    if (cfg_bool_value(root.langs))
    {
        print_all_langs(io, langs);
    }

    diagnostic_list_t common = get_common_diagnostics();

    size_t count = count_diagnostics(&langs) + common.count;

    diag_search_t ctx = {
        .ids = map_optimal(count),
        .diagnostics = typevec_new(sizeof(diagnostic_t), count, arena),
    };

    add_diagnostics(&ctx, common);

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        add_diagnostics(&ctx, lang->diagnostics);
    }

    if (cfg_bool_value(root.diags))
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
    }

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

finish:
    io_close(io);
}
