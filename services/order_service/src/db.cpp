#include <userver/components/component_context.hpp>
#include <string_view>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/json_types.hpp>
#include <userver/storages/postgres/io/optional.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include "db.hpp"
#include "order_status.hpp"

namespace order_service {

OrderDAO::OrderDAO(const userver::components::ComponentContext& context) : 
            pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

userver::storages::postgres::ResultSet OrderDAO::GetOrders(int user_id, bool is_seller) const {
    std::string kSelectOrdersQuery =
        R"~(SELECT id, buyer_id, seller_id, variant_id, quantity, price, status FROM orders p WHERE buyer_id = $1 AND status <> 'cart' ORDER BY id DESC)~";

    if (is_seller) {
        kSelectOrdersQuery =
            R"~(SELECT id, buyer_id, seller_id, variant_id, quantity, price, status FROM orders p WHERE seller_id = $1 AND status <> 'cart' ORDER BY id DESC)~";
    }
    
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectOrdersQuery}},
        user_id
    );
    return result;
}

userver::storages::postgres::ResultSet OrderDAO::GetBasket(int buyer_id) const {

    std::string_view kSelectBasket =
        R"~(SELECT id, buyer_id, seller_id, variant_id, quantity, price, status FROM orders p WHERE buyer_id = $1 AND status = 'cart')~";
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectBasket}},
        buyer_id
    );
    return result;
}

void OrderDAO::InsertOrder(userver::formats::json::Value payload) const {
    std::string kMergeIntoCartQuery = R"~(
    UPDATE orders SET quantity = quantity + 1, price = $3
    WHERE id = (
        SELECT id FROM orders
        WHERE buyer_id = $1 AND variant_id = $2 AND status = 'cart'
        ORDER BY id LIMIT 1
    )
    )~";

    std::string kInsertOrderQuery = R"~(
    INSERT INTO orders (buyer_id, seller_id, variant_id, quantity, price)
    VALUES ($1, $2, $3, 1, $4)
    )~";

    const auto buyer_id = payload["buyerId"].As<std::int64_t>();
    const auto seller_id = payload["sellerId"].As<std::int64_t>();
    const auto variant_id = payload["variantId"].As<std::int64_t>();
    const auto price = payload["price"].As<std::int64_t>();

    try {
        auto transaction = pg_cluster_->Begin(
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::TransactionOptions{}
        );

        const auto merged = transaction.Execute(
            userver::storages::postgres::Query{std::string{kMergeIntoCartQuery}},
            buyer_id,
            variant_id,
            price
        );
        if (merged.RowsAffected() == 0) {
            transaction.Execute(
                userver::storages::postgres::Query{std::string{kInsertOrderQuery}},
                buyer_id,
                seller_id,
                variant_id,
                price
            );
        }
        transaction.Commit();
    } catch (const userver::storages::postgres::IntegrityConstraintViolation&) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Invalid order"}
        );
    }
}

bool OrderDAO::UpdateCartQuantity(std::int64_t buyer_id, std::int64_t variant_id, std::string action) const {
    static constexpr std::string_view kIncreaseQuery = R"~(
    UPDATE orders SET quantity = quantity + 1
    WHERE id = (
        SELECT id FROM orders
        WHERE buyer_id = $1 AND variant_id = $2 AND status = 'cart'
        ORDER BY id LIMIT 1
    )
    )~";
    static constexpr std::string_view kDecreaseQuery = R"~(
    UPDATE orders SET quantity = quantity - 1
    WHERE id = (
        SELECT id FROM orders
        WHERE buyer_id = $1 AND variant_id = $2 AND status = 'cart'
        ORDER BY id LIMIT 1
    ) AND quantity > 1
    )~";
    static constexpr std::string_view kRemoveQuery = R"~(
    DELETE FROM orders
    WHERE id = (
        SELECT id FROM orders
        WHERE buyer_id = $1 AND variant_id = $2 AND status = 'cart'
        ORDER BY id LIMIT 1
    ) AND quantity <= 1
    )~";

    try {
        auto transaction = pg_cluster_->Begin(
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::TransactionOptions{}
        );

        bool found = false;
        if (action == "increase") {
            found = transaction.Execute(
                        userver::storages::postgres::Query{std::string{kIncreaseQuery}}, buyer_id, variant_id
                    ).RowsAffected() > 0;
        } else {
            found = transaction.Execute(
                        userver::storages::postgres::Query{std::string{kDecreaseQuery}}, buyer_id, variant_id
                    ).RowsAffected() > 0;
            if (found == 0) {
                found = transaction.Execute(
                            userver::storages::postgres::Query{std::string{kRemoveQuery}}, buyer_id, variant_id
                        ).RowsAffected() > 0;
            }
        }
        transaction.Commit();
        return found;
    } catch (const userver::storages::postgres::IntegrityConstraintViolation&) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Invalid cart update"}
        );
    }
}

userver::storages::postgres::ResultSet OrderDAO::ResetBasketQuantity(
    std::int64_t buyer_id, std::int64_t variant_id, std::int64_t max_quantity
) const {
    if (max_quantity <= 0) {
        static constexpr std::string_view kRemoveQuery = R"~(
        DELETE FROM orders
        WHERE buyer_id = $1 AND variant_id = $2 AND status = 'cart'
        RETURNING id
        )~";
        return pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::Query{std::string{kRemoveQuery}},
            buyer_id,
            variant_id
        );
    }

    static constexpr std::string_view kClampQuery = R"~(
    UPDATE orders
    SET quantity = LEAST(quantity, $3)
    WHERE buyer_id = $1 AND variant_id = $2 AND status = 'cart'
    RETURNING quantity
    )~";
    return pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kClampQuery}},
        buyer_id,
        variant_id,
        max_quantity
    );
}

std::vector<OrderEvent> OrderDAO::Checkout(std::int64_t buyer_id) const {
    static constexpr std::string_view kCheckoutQuery = R"~(
    UPDATE orders SET status = 'ordered' WHERE buyer_id = $1 AND status = 'cart'
    RETURNING id, seller_id, variant_id, quantity, price
    )~";

    const auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kCheckoutQuery}},
        buyer_id
    );

    std::vector<OrderEvent> events;
    events.reserve(result.Size());
    for (const auto& row : result) {
        events.push_back(OrderEvent{
            row["id"].As<std::int64_t>(),
            buyer_id,
            row["seller_id"].As<std::int64_t>(),
            row["variant_id"].As<std::int64_t>(),
            row["quantity"].As<std::int64_t>(),
            row["price"].As<std::int64_t>(),
            "ordered",
        });
    }
    return events;
}

OrderDAO::StatusChangeResult OrderDAO::ChangeOrderStatus(
    std::int64_t order_id,
    std::int64_t actor_id,
    const std::string& actor_role,
    const std::string& new_status
) const {
    static constexpr std::string_view kSelectForUpdate = R"~(
    SELECT buyer_id, seller_id, variant_id, quantity, price, status FROM orders WHERE id = $1 FOR UPDATE
    )~";
    static constexpr std::string_view kUpdateStatus = R"~(
    UPDATE orders SET status = $2 WHERE id = $1
    )~";

    auto transaction = pg_cluster_->Begin(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::TransactionOptions{}
    );

    const auto result = transaction.Execute(
        userver::storages::postgres::Query{std::string{kSelectForUpdate}}, order_id
    );
    if (result.IsEmpty()) return {StatusChange::kNotFound, std::nullopt};

    const auto row = result[0];
    const auto buyer_id = row["buyer_id"].As<std::int64_t>();
    const auto seller_id = row["seller_id"].As<std::int64_t>();
    const auto owner_id = actor_role == "seller" ? seller_id : buyer_id;
    const auto current_status = row["status"].As<std::string>();

    if (owner_id != actor_id || current_status == "cart") return {StatusChange::kNotFound, std::nullopt};
    if (!IsTransitionAllowed(actor_role, current_status, new_status)) {
        return {StatusChange::kTransitionNotAllowed, std::nullopt};
    }

    transaction.Execute(userver::storages::postgres::Query{std::string{kUpdateStatus}}, order_id, new_status);
    transaction.Commit();

    return {
        StatusChange::kChanged,
        OrderEvent{
            order_id,
            buyer_id,
            seller_id,
            row["variant_id"].As<std::int64_t>(),
            row["quantity"].As<std::int64_t>(),
            row["price"].As<std::int64_t>(),
            new_status,
        },
    };
}

} // namespace order_service