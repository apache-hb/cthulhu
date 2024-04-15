#include "unit/ct-test.h"

#include "base/util.h"
#include "io/io.h"
#include "notify/notify.h"
#include "std/map.h"
#include "std/vector.h"

#include "setup/memory.h"
#include "json/query.h"
#include <stdio.h>

#include "json/json.h"

static const char *const kTestJson =
    "{\n"
    "    \"name\": \"ctu\",\n"
    "    \"version\": 1.0,\n"
    "    \"dependencies\": [\n"
    "        \"base\",\n"
    "        \"std\",\n"
    "        \"meta\"\n"
    "    ]\n"
    "}";


int main(void)
{
    test_install_panic_handler();
    test_install_electric_fence();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("json", arena);

    {
        test_group_t group = test_group(&suite, "parse");
        logger_t *logger = logger_new(arena);
        io_t *io = io_string("test.json", kTestJson, arena);
        json_t *json = json_scan(io, logger, arena);

        GROUP_EXPECT_PASS(group, "json", json != NULL);

        GROUP_EXPECT_PASS(group, "is_object", json->kind == eJsonObject);

        json_t *name = json_map_get(json, "name");
        GROUP_EXPECT_PASS(group, "has name", name != NULL);
        GROUP_EXPECT_PASS(group, "is_name_string", name->kind == eJsonString);
        GROUP_EXPECT_PASS(group, "is_version_float", json_map_get(json, "version")->kind == eJsonFloat);
        GROUP_EXPECT_PASS(group, "is_dependencies_array", json_map_get(json, "dependencies")->kind == eJsonArray);

        text_view_t text = name->string;
        GROUP_EXPECT_PASS(group, "name", text.length == 3);
        GROUP_EXPECT_PASS(group, "name", ctu_strncmp(text.text, "ctu", text.length) == 0);

        float version = json_map_get(json, "version")->real;
        GROUP_EXPECT_PASS(group, "version", version == 1.0f);

        json_t *dependencies = json_map_get(json, "dependencies");
        GROUP_EXPECT_PASS(group, "dependencies", typevec_len(&dependencies->array) == 3);

        GROUP_EXPECT_PASS(group, "dependency_0", json_array_get(dependencies, 0)->kind == eJsonString);
        GROUP_EXPECT_PASS(group, "dependency_1", json_array_get(dependencies, 1)->kind == eJsonString);
        GROUP_EXPECT_PASS(group, "dependency_2", json_array_get(dependencies, 2)->kind == eJsonString);
    }

    {
        test_group_t group = test_group(&suite, "query");
        logger_t *logger = logger_new(arena);
        io_t *io = io_string("test.json", kTestJson, arena);
        json_t *json = json_scan(io, logger, arena);

        GROUP_EXPECT_PASS(group, "json", json != NULL);

        json_t *name = json_query(json, "root.name", logger, arena);
        GROUP_EXPECT_PASS(group, "name", name != NULL);
        GROUP_EXPECT_PASS(group, "name_kind", name->kind == eJsonString);
        GROUP_EXPECT_PASS(group, "name_value", ctu_strncmp(name->string.text, "ctu", name->string.length) == 0);

        json_t *dep1 = json_query(json, "root.dependencies[1]", logger, arena);
        GROUP_EXPECT_PASS(group, "dep1", dep1 != NULL);
        GROUP_EXPECT_PASS(group, "dep_kind", dep1->kind == eJsonString);
        GROUP_EXPECT_PASS(group, "dep_value", ctu_strncmp(dep1->string.text, "std", dep1->string.length) == 0);
    }

    return test_suite_finish(&suite);
}
