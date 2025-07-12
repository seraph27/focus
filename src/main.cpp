#include "crow_all.h"
#include <mutex>
#include <vector>
#include <pqxx/pqxx>

static std::string makeConnectionStr() {
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
struct Task {
    int id;
    std::string name;
    bool done;
};

Task createTask(pqxx::connection& db, const std::string& name) {
    pqxx::work tx{db};
    //exec1 does not work with $1 substitution
    pqxx::result r = tx.exec_params(
        "INSERT INTO tasks(name) VALUES($1) RETURNING id, done",
        name
    );
    tx.commit();
    auto row = r[0];
    return { row[0].as<int>(), name, row[1].as<bool>() };
}

std::vector<Task> listTasks(pqxx::connection& db) {
    pqxx::read_transaction tx{db};
    pqxx::result rows = tx.exec("SELECT id, name, done FROM tasks");
    std::vector<Task> out;
    for (const auto& r : rows) {
      out.push_back({ r[0].as<int>(), r[1].as<std::string>(), r[2].as<bool>() });
    }
    return out;
}

int main() {
    crow::SimpleApp app;
    pqxx::connection db{makeConnectionStr()};

    std::mutex mux;

    //check health
    CROW_ROUTE(app, "/healthz")([](){
        return crow::response{200, "OK"};
    });

    CROW_ROUTE(app, "/tasks").methods("GET"_method)
    ([&](){
        auto tasks = listTasks(db);
        crow::json::wvalue x;
        for (int i = 0; i < (int)tasks.size(); i++) {
            x[i]["id"]   = tasks[i].id;
            x[i]["name"] = tasks[i].name;
            x[i]["done"] = tasks[i].done;
        }
        return crow::response{x};
    });
    
    CROW_ROUTE(app, "/tasks").methods("POST"_method)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body || !body.has("name")) {
            return crow::response{400, "Missing field: title"};
        }
        Task t;
        try {
            t = createTask(db, body["name"].s());
        } catch (const std::exception &e) {
            CROW_LOG_ERROR << "DB exception: " << e.what();
            crow::response r;
            r.code=500;
            r.body = e.what();
            return r;
        }
        crow::json::wvalue res;
        res["id"]    = t.id;
        res["name"] = t.name;
        res["done"]  = t.done;

        auto r = crow::response(res);
        r.code=201;
        return r;
    });
    app.port(8080).multithreaded().run();
    return 0;
}
