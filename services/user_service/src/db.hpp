#pragma once
#include <userver/storages/postgres/cluster.hpp>
#include <optional>
#include <string>
#include <vector>


namespace user_service {
struct UserDAO {
public:
    UserDAO(const userver::components::ComponentContext& context);
    userver::storages::postgres::ResultSet GetMe(std::string email) const;
    void InsertUser(userver::formats::json::Value payload) const;
    void UpdateProfile(
        const std::string& email,
        const std::string& full_name,
        const std::string& phone,
        const std::optional<std::string>& company_name,
        const std::optional<std::string>& tax_id,
        const std::optional<std::string>& address,
        const std::optional<std::string>& description
    ) const;
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};
} // namespace user_service 