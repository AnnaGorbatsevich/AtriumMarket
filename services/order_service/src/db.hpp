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
    bool SetCartQuantity(std::int64_t buyer_id, std::int64_t variant_id, std::int64_t quantity) const;

    std::size_t Checkout(std::int64_t buyer_id) const;

    enum class StatusChange { kChanged, kNotFound, kTransitionNotAllowed };
    StatusChange ChangeOrderStatus(
        std::int64_t order_id,
        std::int64_t actor_id,
        const std::string& actor_role,
        const std::string& new_status
    ) const;
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};
} // namespace order_service 