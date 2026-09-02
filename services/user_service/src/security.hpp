#pragma once

#include <string>

namespace user_service {
    std::string get_password_hash(std::string password);

    bool verify_password(std::string plain_password, std::string hashed_password);

    std::string create_jwt_token(std::string email);

    std::string decode_jwt_token(std::string token);

} // namespace user_service