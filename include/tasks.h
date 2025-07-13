#ifndef TASKS_H
#define TASKS_H

#include <string>
#include <vector>
#include <pqxx/pqxx>

struct Task {
    int id;
    std::string name;
    bool done;
    std::string start_at, end_at;
};

std::string makeConnectionStr();

Task createTask(pqxx::connection& db,
                const std::string& name,
                const std::string& start_at,
                const std::string& end_at);

std::vector<Task> listTasks(pqxx::connection& db);

#endif // TASKS_H
