#include "tasks.h"
#include <cstdlib>

std::string makeConnectionStr() {
    const char* host = std::getenv("DB_HOST");
    const char* port = std::getenv("DB_PORT");
    const char* user = std::getenv("DB_USER");
    const char* pass = std::getenv("DB_PASS");
    const char* name = std::getenv("DB_NAME");

    std::string h = host ? host : "db";
    std::string p = port ? port : "5432";
    std::string u = user ? user : "user";
    std::string pw = pass ? pass : "pass";
    std::string n = name ? name : "focus";

    return "host=" + h + " port=" + p + " dbname=" + n +
           " user=" + u + " password=" + pw;
}

Task createTask(pqxx::connection& db, const std::string& name, const std::string& start_at, const std::string& end_at) {
    pqxx::work tx{db};
    //exec1 does not work with $1 substitution
    pqxx::result r = tx.exec_params(
        "INSERT INTO tasks(name, start_at, end_at) VALUES($1, $2, $3) RETURNING id, name, done, start_at, end_at",
        name, start_at, end_at
    );
    tx.commit();
    auto row = r[0];
    return { row[0].as<int>(), name, row[2].as<bool>(), row[3].as<std::string>(), row[4].as<std::string>() };
}

std::vector<Task> listTasks(pqxx::connection& db) {
    pqxx::read_transaction tx{db};
    pqxx::result rows = tx.exec("SELECT id, name, done, start_at, end_at FROM tasks ORDER BY id");
    std::vector<Task> out;
    for (const auto& r : rows) {
      out.push_back({ r[0].as<int>(), r[1].as<std::string>(), r[2].as<bool>(), r[3].as<std::string>(), r[4].as<std::string>()});
    }
    return out;
}
