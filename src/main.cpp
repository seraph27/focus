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
    std::string start_at, end_at;
};

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
            x[i]["start_at"] = tasks[i].start_at;
            x[i]["end_at"] = tasks[i].end_at;
        }
        return crow::response{x};
    });
    
    CROW_ROUTE(app, "/tasks").methods("POST"_method)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body || !body.has("name") || !body.has("start_at") || !body.has("end_at")) {
            return crow::response{400, "Missing field (name/start_at/end_at)"};
        }
        Task t;
        try {
            t = createTask(db, body["name"].s(), body["start_at"].s(), body["end_at"].s());
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
        res["start_at"] = t.start_at;
        res["end_at"] = t.end_at;

        auto r = crow::response(res);
        r.code=201;
        return r;
    });

    CROW_ROUTE(app, "/tasks/<int>").methods("PATCH"_method)
    ([&](const crow::request& req, int id){
        auto body = crow::json::load(req.body);
        try {
            pqxx::work tx{db};
            pqxx::result r;
            if (body && body.has("done")) {
                r = tx.exec_params("UPDATE tasks SET done=$1 WHERE id=$2", body["done"].b(), id);
            } else {
                r = tx.exec_params("UPDATE tasks SET done=NOT done WHERE id=$1", id);
            }
            if (r.affected_rows() == 0) {
                tx.commit();
                return crow::response{404};
            }
            tx.commit();
            return crow::response{204};
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "DB exception: " << e.what();
            crow::response r; r.code=500; r.body=e.what();
            return r;
        }
    });

    CROW_ROUTE(app, "/tasks/<int>").methods("DELETE"_method)
    ([&](int id){
        try {
            pqxx::work tx{db};
            pqxx::result r = tx.exec_params("DELETE FROM tasks WHERE id=$1", id);
            if (r.affected_rows() == 0) {
                tx.commit();
                return crow::response{404};
            }
            tx.commit();
            return crow::response{204};
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "DB exception: " << e.what();
            crow::response r; r.code=500; r.body=e.what();
            return r;
        }
    });
    app.port(8080).multithreaded().run();
    return 0;
}
