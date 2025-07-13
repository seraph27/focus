#include "crow_all.h"
#include "tasks.h"

#include <mutex>



int main() {
    crow::SimpleApp app;
    pqxx::connection db{makeConnectionStr()};
    
    //pqxx::connection is not thread safe, protect with mutex
    std::mutex mux;

    //check health
    CROW_ROUTE(app, "/healthz")([](){
        return crow::response{200, "OK"};
    });

    CROW_ROUTE(app, "/tasks").methods("GET"_method)
    ([&](){
        std::lock_guard<std::mutex> guard(mux);
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
        std::lock_guard<std::mutex> guard(mux);
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
        std::lock_guard<std::mutex> guard(mux);
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
        std::lock_guard<std::mutex> guard(mux);
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
