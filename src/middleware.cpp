#include "middleware.h"
#include "auth.h"

void AuthMiddleware::before_handle(crow::request& req, crow::response& res, context& ctx) {
    auto auth_header = req.get_header_value("Authorization");
    if (auth_header.empty() || auth_header.find("Bearer ") != 0) {
        res.code = 401;
        res.end("Unauthorized: No token provided");
        return;
    }

    std::string token = auth_header.substr(7); // Skip "Bearer "
    if (!verify_jwt(token)) {
        res.code = 401;
        res.end("Unauthorized: Invalid token");
        return;
    }
}

void AuthMiddleware::after_handle(crow::request& req, crow::response& res, context& ctx) {
    // This is where you could add any logic to be executed after the request has been handled.
}
