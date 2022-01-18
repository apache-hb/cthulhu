#include "json.h"

#include "cJSON.h"

bool json_build(reports_t *reports, module_t *mod, const char *path) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "path", path);
    cJSON_AddStringToObject(root, "name", mod->name);

    report(reports, NOTE, NULL, "json: %s", cJSON_Print(root));
    return true;
}
