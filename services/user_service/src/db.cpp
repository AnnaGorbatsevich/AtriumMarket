#include <userver/components/component_context.hpp>
#include <string_view>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/json_types.hpp>
#include <userver/storages/postgres/io/optional.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include "db.hpp"
#include "security.hpp"

namespace user_service {

std::optional<std::string> OptionalField(const userver::formats::json::Value& payload, std::string_view field) {
    auto value = payload[std::string{field}].As<std::string>({});
    if (value.empty()) return std::nullopt;
    return value;
}

UserDAO::UserDAO(const userver::components::ComponentContext& context) : 
            pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

userver::v3_2_rc::storages::postgres::ResultSet UserDAO::GetMe(std::string email) const {

    std::string_view kSelectProductsBySellerQuery = R"~(
    SELECT id, full_name, password_hash, role::text FROM users WHERE email = $1
    )~";
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectProductsBySellerQuery}},
        email
    );
    return result;
}

void UserDAO::InsertUser(userver::formats::json::Value payload) const {
    std::string kInsertUserQuery = R"~(
    INSERT INTO users (role, full_name, company_name, email, phone, tax_id, address, description, password_hash)
    VALUES ($1::user_role, $2, $3, $4, $5, $6, $7, $8, $9)
    )~";

    const auto email = payload["email"].As<std::string>();
    const auto password_hash = get_password_hash(payload["password"].As<std::string>());

    try {
        pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::Query{std::string{kInsertUserQuery}},
            payload["role"].As<std::string>(),
            payload["fullName"].As<std::string>(),
            OptionalField(payload, "companyName"),
            email,
            payload["phone"].As<std::string>(),
            OptionalField(payload, "taxId"),
            OptionalField(payload, "address"),
            OptionalField(payload, "description"),
            password_hash
        );
    } catch (const userver::storages::postgres::UniqueViolation&) {
        throw userver::server::handlers::ClientError(userver::server::handlers::ExternalBody{"Email already registered"}
        );
    }
}

} // namespace listing_service