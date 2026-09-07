#include <userver/components/component_context.hpp>
#include <string_view>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/json_types.hpp>
#include <userver/storages/postgres/io/optional.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include "db.hpp"

namespace user_service {

UserDAO::UserDAO(const userver::components::ComponentContext& context) : 
            pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

userver::v3_2_rc::storages::postgres::ResultSet UserDAO::GetMe(std::string email) const {

    std::string_view kSelectProductsBySellerQuery = R"~(
    SELECT id, full_name, role::text FROM users WHERE email = $1
    )~";
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectProductsBySellerQuery}},
        email
    );
    return result;
}

} // namespace listing_service