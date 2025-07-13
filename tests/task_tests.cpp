#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "tasks.h"

TEST_CASE("createTask and listTasks", "[db]") {
    pqxx::connection db{makeConnectionStr()};
    pqxx::work clear{db};
    clear.exec("TRUNCATE TABLE tasks RESTART IDENTITY");
    clear.commit();

    Task t = createTask(db, "UnitTest", "2025-07-12T00:00:00Z", "2025-07-12T01:00:00Z");
    REQUIRE(t.id == 1);
    REQUIRE(t.name == "UnitTest");
    REQUIRE(t.done == false);

    auto list = listTasks(db);
    REQUIRE(list.size() == 1);
    REQUIRE(list[0].id == t.id);
    REQUIRE(list[0].name == t.name);
}
