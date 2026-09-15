#include "get_orders.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <optional>
#include <string_view>

#include <fmt/format.h>

#include <userver/components/component_context.hpp>
#include <userver/formats/common/type.hpp>
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

namespace order_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

}  // namespace

GetOrderHandler::GetOrderHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      db_dao_(context) {}

std::string GetOrderHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto& http_response = request.GetHttpResponse();
    SetCorsHeaders(http_response);

    if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
        return {};
    }

    std::int64_t user_id = 0;
    bool is_seller = true;

    if (!request.HasArg("sellerId") && !request.HasArg("buyerId")) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Missing required query parameter: sellerId or buyerId"}
        );
    } else if (request.HasArg("sellerId")) {
        is_seller = true;
        try {
            user_id = std::stoll(request.GetArg("sellerId"));
        } catch (const std::exception&) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{"sellerId must be an integer"}
            );
        }
    } else {
        is_seller = false;
        try {
            user_id = std::stoll(request.GetArg("buyerId"));
        } catch (const std::exception&) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{"buyerId must be an integer"}
            );
        }
    }


    auto result = db_dao_.GetOrders(user_id, is_seller);

    userver::formats::json::ValueBuilder response_body{userver::formats::common::Type::kArray};
    for (const auto& row : result) {
        userver::formats::json::ValueBuilder product;
        product["id"] = row["id"].As<std::int64_t>();
        product["buyerId"] = row["buyer_id"].As<std::int64_t>();
        product["sellerId"] = row["seller_id"].As<std::int64_t>();
        product["variantId"] = row["variant_id"].As<std::int64_t>();
        product["quantity"] = row["quantity"].As<std::int64_t>();
        product["price"] = row["price"].As<std::int64_t>();
        response_body.PushBack(std::move(product));
    }

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace order_service
