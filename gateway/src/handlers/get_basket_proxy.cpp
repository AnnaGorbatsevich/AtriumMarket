#include "get_basket_proxy.hpp"

#include <chrono>

#include <userver/clients/http/component.hpp>
#include <userver/clients/http/error.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/server/http/http_status.hpp>

#include "../config.hpp"

namespace gateway {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Authorization, Content-Type"});
}

}  // namespace

GetBasketProxyHandler::GetBasketProxyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      http_requests_(context) {}

std::string GetBasketProxyHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto& http_response = request.GetHttpResponse();
    SetCorsHeaders(http_response);

    if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
        return {};
    }

    userver::formats::json::Value identity;
    try {
        auto me_response = http_requests_.GetMe(request);

        if (me_response->status_code() != 200) {
            http_response.SetStatus(static_cast<userver::server::http::HttpStatus>(me_response->status_code()));
            http_response.SetContentType(userver::http::content_type::kApplicationJson);
            return me_response->body();
        }

        identity = userver::formats::json::FromString(me_response->body());
    } catch (const userver::clients::http::BaseException&) {
        throw userver::server::handlers::CustomHandlerException(
            userver::server::handlers::HandlerErrorCode::kBadGateway,
            userver::server::handlers::ExternalBody{"user-service is unavailable"}
        );
    }

    const auto buyer_id = identity["id"].As<std::int64_t>();

    try {
        auto upstream_response = http_requests_.GetBasket(buyer_id);

        userver::formats::json::Value result = userver::formats::json::FromString(upstream_response->body());
        userver::formats::json::ValueBuilder response_body{userver::formats::common::Type::kArray};
        for (const auto& row : result) {
            const auto variant_id = row["variantId"].As<std::int64_t>();
            const auto availability = http_requests_.GetAvailability(variant_id);
            auto quantity = row["quantity"].As<std::int64_t>();

            if (quantity > availability) {
                auto reset_response = http_requests_.ResetBasketQuantity(buyer_id, variant_id, availability);
                if (reset_response->status_code() == 200) {
                    const auto reset_body = userver::formats::json::FromString(reset_response->body());
                    if (reset_body["removed"].As<bool>(false)) {
                        continue;
                    }
                    quantity = reset_body["quantity"].As<std::int64_t>();
                }
            }

            userver::formats::json::ValueBuilder product;
            product["id"] = row["id"].As<std::int64_t>();
            product["buyerId"] = row["buyerId"].As<std::int64_t>();
            product["sellerId"] = row["sellerId"].As<std::int64_t>();
            product["variantId"] = variant_id;
            product["quantity"] = quantity;
            product["price"] = row["price"].As<std::int64_t>();
            product["availability"] = availability;
            response_body.PushBack(std::move(product));
        }

        http_response.SetStatus(static_cast<userver::server::http::HttpStatus>(upstream_response->status_code()));
        http_response.SetContentType(userver::http::content_type::kApplicationJson);
        return userver::formats::json::ToString(response_body.ExtractValue());
    } catch (const userver::clients::http::BaseException&) {
        throw userver::server::handlers::CustomHandlerException(
            userver::server::handlers::HandlerErrorCode::kBadGateway,
            userver::server::handlers::ExternalBody{"order-service is unavailable"}
        );
    }
}

}  // namespace gateway
