// pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster())
#include <userver/components/component_context.hpp>
#include <string_view>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/json_types.hpp>
#include <userver/storages/postgres/io/optional.hpp>
#include "db.hpp"

namespace listing_service {
ListingDAO::ListingDAO(const userver::components::ComponentContext& context) : 
            pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

userver::v3_2_rc::storages::postgres::ResultSet ListingDAO::GetProducts(int seller_id) const {

    std::string_view kSelectProductsBySellerQuery = R"~(
    SELECT
        p.id,
        p.name,
        p.description,
        p.category_id,
        c.name AS category_name,
        p.status::text AS status,
        p.created_at,
        COALESCE(
            (SELECT json_agg(
                        json_build_object('id', v.id, 'price', v.price, 'quantity', v.quantity, 'options', v.options)
                        ORDER BY v.id
                    )
            FROM variants v WHERE v.product_id = p.id),
            '[]'
        )::jsonb AS variants
    FROM products p
    LEFT JOIN categories c ON c.id = p.category_id
    WHERE p.seller_id = $1 AND p.status != 'deleted'
    ORDER BY p.created_at DESC
    )~";
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectProductsBySellerQuery}},
        seller_id
    );
    return result;
}

userver::v3_2_rc::storages::postgres::ResultSet ListingDAO::GetCategories() const {

    std::string_view kSelectProductsBySellerQuery = R"~(
    SELECT id, name, parent_id FROM categories ORDER BY name
    )~";
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectProductsBySellerQuery}}
    );
    return result;
}

userver::v3_2_rc::storages::postgres::ResultSet ListingDAO::GetCatalog(std::optional<long int> category_id) const {

    std::string_view kSelectProductsBySellerQuery = R"~(
        SELECT
        p.id,
        p.seller_id,
        p.name,
        p.description,
        p.category_id,
        c.name AS category_name,
        p.created_at,
        COALESCE(
            (SELECT json_agg(
                        json_build_object('id', v.id, 'price', v.price, 'quantity', v.quantity, 'options', v.options)
                        ORDER BY v.id
                    )
            FROM variants v WHERE v.product_id = p.id),
            '[]'
        )::jsonb AS variants
    FROM products p
    LEFT JOIN categories c ON c.id = p.category_id
    WHERE p.status = 'active' AND ($1::bigint IS NULL OR p.category_id = $1)
    ORDER BY p.created_at DESC
    )~";
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectProductsBySellerQuery}},
        category_id
    );
    return result;
}

} // namespace listing_service