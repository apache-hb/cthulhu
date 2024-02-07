#include "io/io.h"
#include "notify/notify.h"
#include "std/map.h"
#include "unit/ct-test.h"

#include "setup/memory.h"

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
        GROUP_EXPECT_PASS(group, "is_name_string", map_get(json->object, "name")->kind == eJsonString);
        GROUP_EXPECT_PASS(group, "is_version_float", json["version"].is_float());
        GROUP_EXPECT_PASS(group, "is_dependencies_array", json["dependencies"].is_array());

        text_view_t name = json["name"].as_string();
        GROUP_EXPECT_PASS(group, "name", name.length == 4);

        GROUP_EXPECT_PASS(group, "name", ctu_strncmp(name.text, "ctu", name.length) == 0);

        float version = json["version"].as_float();
        GROUP_EXPECT_PASS(group, "version", version == 1.0f);

        json::Array dependencies = json["dependencies"].as_array();
        GROUP_EXPECT_PASS(group, "dependencies", dependencies.length() == 3);

        GROUP_EXPECT_PASS(group, "dependency_0", dependencies[0].is_string());
        GROUP_EXPECT_PASS(group, "dependency_1", dependencies[1].is_string());
        GROUP_EXPECT_PASS(group, "dependency_2", dependencies[2].is_string());
    }

}
