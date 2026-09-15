#include <userver/components/component_context.hpp>
#include <string_view>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/json_types.hpp>
#include <userver/storages/postgres/io/optional.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include "db.hpp"

namespace order_service {

OrderDAO::OrderDAO(const userver::components::ComponentContext& context) : 
            pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

userver::v3_2_rc::storages::postgres::ResultSet OrderDAO::GetOrders(int user_id, bool is_seller) const {
    std::string kSelectOrdersQuery =
        R"~(SELECT id, buyer_id, seller_id, variant_id, quantity, price, status FROM orders p WHERE buyer_id = $1)~";

    if (is_seller) {
        kSelectOrdersQuery =
            R"~(SELECT id, buyer_id, seller_id, variant_id, quantity, price, status FROM orders p WHERE seller_id = $1 AND status <> 'cart')~";
    }
    
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectOrdersQuery}},
        user_id
    );
    return result;
}

userver::v3_2_rc::storages::postgres::ResultSet OrderDAO::GetBasket(int buyer_id) const {

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
    std::string kInsertOrderQuery = R"~(
    INSERT INTO orders (buyer_id, seller_id, variant_id, quantity, price)
    VALUES ($1, $2, $3, $4, $5)
    )~";

    const auto buyer_id = payload["buyerId"].As<std::int64_t>();
    const auto seller_id = payload["sellerId"].As<std::int64_t>();
    const auto variant_id = payload["variantId"].As<std::int64_t>();
    const auto quantity = payload["quantity"].As<std::int64_t>();
    const auto price = payload["price"].As<std::int64_t>();

    try {
        auto transaction = pg_cluster_->Begin(
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::TransactionOptions{}
        );

        transaction.Execute(
            userver::storages::postgres::Query{std::string{kInsertOrderQuery}},
            buyer_id,
            seller_id,
            variant_id,
            quantity,
            price
        );
        transaction.Commit();
    } catch (const userver::storages::postgres::IntegrityConstraintViolation&) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Invalid order"}
        );
    }
}

} // namespace order_service