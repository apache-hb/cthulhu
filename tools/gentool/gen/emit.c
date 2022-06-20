#include "emit.h"

#include "report/report.h"

#define CONFIG_NAME "name"
#define CONFIG_VERSION "version"
#define CONFIG_ID "id"
#define CONFIG_DESC "desc"
#define CONFIG_EXTS "exts"

static const char *get_string(reports_t *reports, map_t *map, const char *field)
{
    const char *value = map_get(map, field);
    
    if (value == NULL) 
    {
        report(reports, ERROR, node_invalid(), "missing field '%s'", field);
        return "";
    }

    return value;
}

void emit_header(reports_t *reports, file_t file, const ast_t *root)
{
    map_t *config = root->config->fields;

    const char *lang = get_string(reports, config, CONFIG_NAME);
}
