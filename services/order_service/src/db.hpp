#pragma once
#include <userver/storages/postgres/cluster.hpp>
#include <vector>


namespace order_service {
struct OrderDAO {
public:
    OrderDAO(const userver::components::ComponentContext& context);
    userver::storages::postgres::ResultSet GetOrders(int user_id, bool is_seller) const;
    userver::storages::postgres::ResultSet GetBasket(int buyer_id) const;
    void InsertOrder(userver::formats::json::Value payload) const;
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};
} // namespace order_service 