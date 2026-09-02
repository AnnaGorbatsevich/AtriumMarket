#include "security.hpp"

#include <chrono>
#include <cstdlib>
#include <memory>
#include <stdexcept>

#include <crypt.h>

#include <userver/crypto/base64.hpp>
#include <userver/crypto/hash.hpp>
#include <userver/formats/json.hpp>

namespace user_service {

namespace {

constexpr unsigned kTokenLifetimeSeconds = 3600;

const std::string& JwtSecret() {
    static const std::string secret = [] {
        const char* value = std::getenv("JWT_SECRET");
        if (value == nullptr || value[0] == '\0') {
            throw std::runtime_error("JWT_SECRET environment variable is not set");
        }
        return std::string{value};
    }();
    return secret;
}

std::string Base64UrlEncodeJson(const userver::formats::json::Value& value) {
    return userver::crypto::base64::Base64UrlEncode(
        userver::formats::json::ToString(value), userver::crypto::base64::Pad::kWithout
    );
}

std::string SignHs256(std::string_view signing_input) {
    return userver::crypto::base64::Base64UrlEncode(
        userver::crypto::hash::HmacSha256(
            JwtSecret(), signing_input, userver::crypto::hash::OutputEncoding::kBinary
        ),
        userver::crypto::base64::Pad::kWithout
    );
}

unsigned NowUnixSeconds() {
    return static_cast<unsigned>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()
    );
}

}  // namespace

std::string get_password_hash(std::string password) {
    char salt_buf[CRYPT_GENSALT_OUTPUT_SIZE];
    const char* setting = crypt_gensalt_rn(nullptr, 0, nullptr, 0, salt_buf, sizeof(salt_buf));
    if (setting == nullptr) {
        throw std::runtime_error("Failed to generate password salt");
    }

    auto data = std::make_unique<crypt_data>();
    *data = {};
    const char* hashed = crypt_r(password.c_str(), setting, data.get());
    if (hashed == nullptr || hashed[0] == '*') {
        throw std::runtime_error("Failed to hash password");
    }
    return std::string{hashed};
}

bool verify_password(std::string plain_password, std::string hashed_password) {
    auto data = std::make_unique<crypt_data>();
    *data = {};
    const char* result = crypt_r(plain_password.c_str(), hashed_password.c_str(), data.get());
    if (result == nullptr || result[0] == '*') {
        return false;
    }
    return hashed_password == result;
}

std::string create_jwt_token(std::string email) {
    userver::formats::json::ValueBuilder header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";

    userver::formats::json::ValueBuilder payload;
    payload["email"] = email;
    payload["exp"] = NowUnixSeconds() + kTokenLifetimeSeconds;

    const auto signing_input = Base64UrlEncodeJson(header.ExtractValue()) + "." +
                                Base64UrlEncodeJson(payload.ExtractValue());

    return signing_input + "." + SignHs256(signing_input);
}

std::string decode_jwt_token(std::string token) {
    const auto first_dot = token.find('.');
    const auto second_dot = first_dot == std::string::npos ? std::string::npos : token.find('.', first_dot + 1);
    if (first_dot == std::string::npos || second_dot == std::string::npos) {
        throw std::runtime_error("Malformed JWT token");
    }

    const auto signing_input = token.substr(0, second_dot);
    const auto payload_b64 = token.substr(first_dot + 1, second_dot - first_dot - 1);
    const auto signature_b64 = token.substr(second_dot + 1);

    if (SignHs256(signing_input) != signature_b64) {
        throw std::runtime_error("Invalid JWT signature");
    }

    const auto payload = userver::formats::json::FromString(userver::crypto::base64::Base64UrlDecode(payload_b64));

    if (NowUnixSeconds() >= payload["exp"].As<unsigned>()) {
        throw std::runtime_error("JWT token expired");
    }

    return payload["email"].As<std::string>();
}

}  // namespace user_service
