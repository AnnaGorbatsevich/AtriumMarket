#include "add_product.hpp"

#include <optional>
#include <string_view>

#include <fmt/format.h>

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/exceptions.hpp>
#include <userver/storages/postgres/io/json_types.hpp>
#include <userver/storages/postgres/io/optional.hpp>
#include <userver/storages/postgres/transaction.hpp>

namespace listing_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"POST, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

constexpr std::string_view kRequiredFields[] = {"sellerId", "name", "price", "quantity"};

void ValidatePayload(const userver::formats::json::Value& payload) {
    for (const auto field : kRequiredFields) {
        const auto& value = payload[std::string{field}];
        if (value.IsMissing() || value.IsNull()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{fmt::format("Missing required field: {}", field)}
            );
        }
    }
    if (payload["name"].As<std::string>({}).empty()) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Missing required field: name"}
        );
    }
    if (payload["price"].As<int>(-1) < 0) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"price must be a non-negative integer"}
        );
    }
    if (payload["quantity"].As<int>(-1) < 0) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"quantity must be a non-negative integer"}
        );
    }
}

userver::formats::json::Value ObjectOrEmpty(const userver::formats::json::Value& payload, std::string_view field) {
    const auto& value = payload[std::string{field}];
    if (value.IsMissing() || value.IsNull()) {
        return userver::formats::json::MakeObject();
    }
    return value;
}

constexpr std::string_view kInsertProductQuery = R"~(
INSERT INTO products (seller_id, name, description, category_id, attributes, status)
VALUES ($1, $2, $3, $4, $5, 'active')
RETURNING id
)~";

constexpr std::string_view kInsertVariantQuery = R"~(
INSERT INTO variants (product_id, price, quantity, options)
VALUES ($1, $2, $3, $4)
)~";

}  // namespace

AddProductHandler::AddProductHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

std::string AddProductHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto& http_response = request.GetHttpResponse();
    SetCorsHeaders(http_response);

    if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
        return {};
    }

    userver::formats::json::Value payload;
    try {
        payload = userver::formats::json::FromString(request.RequestBody());
    } catch (const userver::formats::json::Exception&) {
        throw userver::server::handlers::ClientError(userver::server::handlers::ExternalBody{"Invalid JSON body"});
    }

    ValidatePayload(payload);

    const auto seller_id = payload["sellerId"].As<std::int64_t>();
    const auto name = payload["name"].As<std::string>();
    const auto description = payload["description"].As<std::optional<std::string>>();
    const auto category_id = payload["categoryId"].As<std::optional<std::int64_t>>();
    const auto attributes = ObjectOrEmpty(payload, "attributes");
    const auto price = payload["price"].As<int>();
    const auto quantity = payload["quantity"].As<int>();
    const auto options = ObjectOrEmpty(payload, "options");

    std::int64_t product_id = 0;
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
            attributes
        );
        product_id = product_result[0]["id"].As<std::int64_t>();

        transaction.Execute(
            userver::storages::postgres::Query{std::string{kInsertVariantQuery}}, product_id, price, quantity, options
        );

        transaction.Commit();
    } catch (const userver::storages::postgres::IntegrityConstraintViolation&) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Invalid categoryId or product data"}
        );
    }

    userver::formats::json::ValueBuilder response_body;
    response_body["status"] = "ok";
    response_body["productId"] = product_id;

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace listing_service
