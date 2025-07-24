#pragma once

#include <string>

// NOTE: This is a boilerplate function.
// You should implement the actual password hashing logic using bcrypt.
std::string hash_password(const std::string& password);

// NOTE: This is a boilerplate function.
// You should implement the actual password verification logic using bcrypt.
bool verify_password(const std::string& password, const std::string& hash);

// NOTE: This is a boilerplate function.
// You should implement the actual JWT generation logic.
std::string generate_jwt(const std::string& user_id);

// NOTE: This is a boilerplate function.
// You should implement the actual JWT verification logic.
bool verify_jwt(const std::string& token);
