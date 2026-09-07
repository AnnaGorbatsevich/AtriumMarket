// pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster())
#include <userver/components/component_context.hpp>
#include <string_view>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/json_types.hpp>
#include <userver/storages/postgres/io/optional.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include "db.hpp"

namespace listing_service {

userver::formats::json::Value ObjectOrEmpty(const userver::formats::json::Value& payload, std::string_view field) {
    const auto& value = payload[std::string{field}];
    if (value.IsMissing() || value.IsNull()) {
        return userver::formats::json::MakeObject();
    }
    return value;
}

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

void ListingDAO::InsertProduct(userver::formats::json::Value payload) const {
    std::string kInsertProductQuery = R"~(
    INSERT INTO products (seller_id, name, description, category_id, attributes, status)
    VALUES ($1, $2, $3, $4, $5, $6::product_status)
    RETURNING id
    )~";

    std::string kInsertVariantQuery = R"~(
    INSERT INTO variants (product_id, price, quantity, options)
    VALUES ($1, $2, $3, $4)
    RETURNING id
    )~";
    const auto seller_id = payload["sellerId"].As<std::int64_t>();
    const auto name = payload["name"].As<std::string>();
    const auto description = payload["description"].As<std::optional<std::string>>();
    const auto category_id = payload["categoryId"].As<std::optional<std::int64_t>>();
    const auto attributes = ObjectOrEmpty(payload, "attributes");
    const auto status = payload["status"].As<std::string>("active");
    const auto& variants = payload["variants"];

    std::int64_t product_id = 0;
    userver::formats::json::ValueBuilder variant_ids{userver::formats::common::Type::kArray};
    try {
        auto transaction = pg_cluster_->Begin(
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::TransactionOptions{}
        );

        auto product_result = transaction.Execute(
            userver::storages::postgres::Query{std::string{kInsertProductQuery}},
            seller_id,
            name,
            description,
            category_id,
            attributes,
            status
        );
        product_id = product_result[0]["id"].As<std::int64_t>();

        for (const auto& variant : variants) {
            const auto price = variant["price"].As<int>();
            const auto quantity = variant["quantity"].As<int>();
            const auto options = ObjectOrEmpty(variant, "options");

            auto variant_result = transaction.Execute(
                userver::storages::postgres::Query{std::string{kInsertVariantQuery}},
                product_id,
                price,
                quantity,
                options
            );
            variant_ids.PushBack(variant_result[0]["id"].As<std::int64_t>());
        }

        transaction.Commit();
    } catch (const userver::storages::postgres::IntegrityConstraintViolation&) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Invalid categoryId or product data"}
        );
    }
}

} // namespace listing_service