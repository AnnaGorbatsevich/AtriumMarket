#pragma once
#include <userver/storages/postgres/cluster.hpp>
#include <vector>


namespace user_service {
struct UserDAO {
public:
    UserDAO(const userver::components::ComponentContext& context);
    userver::v3_2_rc::storages::postgres::ResultSet GetMe(std::string email) const;
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};
} // namespace user_service 