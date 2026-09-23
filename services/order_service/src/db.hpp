#pragma once
#include <optional>
#include <vector>
#include <userver/storages/postgres/cluster.hpp>

#include "events.hpp"

namespace order_service {
struct OrderDAO {
public:
    OrderDAO(const userver::components::ComponentContext& context);
    userver::storages::postgres::ResultSet GetOrders(int user_id, bool is_seller) const;
    userver::storages::postgres::ResultSet GetBasket(int buyer_id) const;
    void InsertOrder(userver::formats::json::Value payload) const;
    bool UpdateCartQuantity(std::int64_t buyer_id, std::int64_t variant_id, std::string action) const;
    userver::storages::postgres::ResultSet ResetBasketQuantity(
        std::int64_t buyer_id, std::int64_t variant_id, std::int64_t max_quantity
    ) const;

    std::vector<OrderEvent> Checkout(std::int64_t buyer_id) const;

    enum class StatusChange { kChanged, kNotFound, kTransitionNotAllowed };
    struct StatusChangeResult {
        StatusChange result;
        std::optional<OrderEvent> event;
    };
    StatusChangeResult ChangeOrderStatus(
        std::int64_t order_id,
        std::int64_t actor_id,
        const std::string& actor_role,
        const std::string& new_status
    ) const;
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};
} // namespace order_service