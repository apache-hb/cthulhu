#include "unit/ct-test.h"

#include "base/util.h"
#include "io/io.h"
#include "notify/notify.h"
#include "std/set.h"
#include "std/str.h"
#include "setup/memory.h"

#include "json/json.hpp"

namespace json = ctu::json;

static const char *const kTestJson = R"(
{
    "name": "ctu",
    "version": 1.0,
    "dependencies": [
        "ctu",
        "unit",
        "json"
    ]
}
)";

int main(void)
{
    test_install_panic_handler();
    test_install_electric_fence();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("json", arena);

    notify_rules_t rules = {
        .warnings_as_errors = set_new(1, kTypeInfoPtr, arena),
        .ignored_warnings = set_new(1, kTypeInfoPtr, arena),
    };

    {
        test_group_t group = test_group(&suite, "parse");
        json::JsonParser parser{arena};

        io_t *io = io_string("test.json", kTestJson, arena);

        json::Json json = parser.parse(io);

        logger_t *logger = parser.get_logger();
        GROUP_EXPECT_PASS(group, "no_errors", !logger_has_errors(logger, rules));

        GROUP_EXPECT_PASS(group, "is_valid", json.is_valid());
        GROUP_EXPECT_PASS(group, "is_object", json.is_object());
        GROUP_EXPECT_PASS(group, "is_name_string", json["name"].is_string());
        GROUP_EXPECT_PASS(group, "is_version_float", json["version"].is_float());
        GROUP_EXPECT_PASS(group, "is_dependencies_array", json["dependencies"].is_array());

        std::string_view name = json["name"].as_string();
        GROUP_EXPECT_PASS(group, "name", name.length() == 3);

        GROUP_EXPECT_PASS(group, "name", name == "ctu");

        float version = json["version"].as_float();
        GROUP_EXPECT_PASS(group, "version", version == 1.0f);

        json::Array dependencies = json["dependencies"].as_array();
        GROUP_EXPECT_PASS(group, "dependencies", dependencies.length() == 3);

        GROUP_EXPECT_PASS(group, "dependency_0", dependencies[0].is_string());
        GROUP_EXPECT_PASS(group, "dependency_1", dependencies[1].is_string());
        GROUP_EXPECT_PASS(group, "dependency_2", dependencies[2].is_string());
    }

    return test_suite_finish(&suite);
}
