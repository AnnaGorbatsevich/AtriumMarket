#include "products.hpp"

#include <chrono>
#include <optional>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/common/type.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/json_types.hpp>
#include <userver/storages/postgres/io/optional.hpp>

namespace listing_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

constexpr std::string_view kSelectProductsBySellerQuery = R"~(
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

}  // namespace

ProductsHandler::ProductsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

std::string ProductsHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto& http_response = request.GetHttpResponse();
    SetCorsHeaders(http_response);

    if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
        return {};
    }

    if (!request.HasArg("sellerId")) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Missing required query parameter: sellerId"}
        );
    }

    std::int64_t seller_id = 0;
    try {
        seller_id = std::stoll(request.GetArg("sellerId"));
    } catch (const std::exception&) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"sellerId must be an integer"}
        );
    }

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectProductsBySellerQuery}},
        seller_id
    );

    userver::formats::json::ValueBuilder response_body{userver::formats::common::Type::kArray};
    for (const auto& row : result) {
        userver::formats::json::ValueBuilder product;
        product["id"] = row["id"].As<std::int64_t>();
        product["name"] = row["name"].As<std::string>();
        product["description"] = row["description"].As<std::optional<std::string>>();
        product["categoryId"] = row["category_id"].As<std::optional<std::int64_t>>();
        product["categoryName"] = row["category_name"].As<std::optional<std::string>>();
        product["status"] = row["status"].As<std::string>();
        product["createdAt"] = row["created_at"].As<std::chrono::system_clock::time_point>();
        product["variants"] = row["variants"].As<userver::formats::json::Value>();
        response_body.PushBack(std::move(product));
    }

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace listing_service
