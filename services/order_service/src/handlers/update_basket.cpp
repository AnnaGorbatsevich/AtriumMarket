#include "update_basket.hpp"

#include <fmt/format.h>

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>

namespace order_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"POST, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

constexpr std::string_view kRequiredFields[] = {"buyerId", "variantId", "action"};
constexpr std::string_view kIncrease = "increase";
constexpr std::string_view kDecrease = "decrease";

void ValidatePayload(const userver::formats::json::Value& payload) {
    for (const auto field : kRequiredFields) {
        const auto& value = payload[std::string{field}];
        if (value.IsMissing() || value.IsNull()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{fmt::format("Missing required field: {}", field)}
            );
        }
    }
    const auto action = payload["action"].As<std::string>();
    if (action != kIncrease && action != kDecrease) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"action must be increase or decrease"}
        );
    }
}

}  // namespace

UpdateBasketHandler::UpdateBasketHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      db_dao_(context) {}

std::string UpdateBasketHandler::HandleRequestThrow(
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

    const auto action = payload["action"].As<std::string>();
    const bool found = db_dao_.UpdateCartQuantity(
        payload["buyerId"].As<std::int64_t>(),
        payload["variantId"].As<std::int64_t>(),
        action
    );
    if (!found) {
        throw userver::server::handlers::CustomHandlerException(
            userver::server::handlers::HandlerErrorCode::kResourceNotFound,
            userver::server::handlers::ExternalBody{"Item is not in the cart"}
        );
    }

    userver::formats::json::ValueBuilder response_body;
    response_body["status"] = "ok";

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace order_service
