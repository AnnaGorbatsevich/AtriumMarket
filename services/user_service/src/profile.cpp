#include "profile.hpp"
#include "security.hpp"

#include <optional>
#include <string_view>

#include <userver/formats/json.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/io/optional.hpp>

namespace user_service {

namespace {

constexpr std::string_view kBearerPrefix = "Bearer ";

}  // namespace

std::string AuthenticatedEmail(const userver::server::http::HttpRequest& request) {
    const auto& auth_header = request.GetHeader("Authorization");
    if (!auth_header.starts_with(kBearerPrefix)) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Missing bearer token"}
        );
    }

    try {
        return decode_jwt_token(auth_header.substr(kBearerPrefix.size()));
    } catch (const std::exception&) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Invalid or expired token"}
        );
    }
}

userver::formats::json::Value ProfileToJson(const std::string& email, const userver::storages::postgres::Row& row) {
    userver::formats::json::ValueBuilder profile;
    profile["id"] = row["id"].As<std::int64_t>();
    profile["email"] = email;
    profile["fullName"] = row["full_name"].As<std::string>();
    profile["role"] = row["role"].As<std::string>();
    profile["phone"] = row["phone"].As<std::string>();
    profile["companyName"] = row["company_name"].As<std::optional<std::string>>();
    profile["taxId"] = row["tax_id"].As<std::optional<std::string>>();
    profile["address"] = row["address"].As<std::optional<std::string>>();
    profile["description"] = row["description"].As<std::optional<std::string>>();
    return profile.ExtractValue();
}

}  // namespace user_service
