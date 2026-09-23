#include "decrease_availability.hpp"

#include <string_view>

#include <fmt/format.h>

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/storages/postgres/component.hpp>

namespace listing_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"POST, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

constexpr std::string_view kRequiredFields[] = {"variantId", "quantity"};

void ValidatePayload(const userver::formats::json::Value& payload) {
    for (const auto field : kRequiredFields) {
        const auto& value = payload[std::string{field}];
        if (value.IsMissing() || value.IsNull()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{fmt::format("Missing required field: {}", field)}
            );
        }
    }
}

}  // namespace

DecreaseAvailabilityHandler::DecreaseAvailabilityHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      db_dao_(context) {}

std::string DecreaseAvailabilityHandler::HandleRequestThrow(
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

    const auto variant_id = payload["variantId"].As<std::int64_t>();
    const auto quantity = payload["quantity"].As<std::int64_t>();

    auto result = db_dao_.DecreaseAvailability(variant_id, quantity);
    if (result.IsEmpty()) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Invalid variantId"}
        );
    }

    userver::formats::json::ValueBuilder response_body;
    response_body["status"] = "ok";
    response_body["remaining"] = result[0]["remaining"].As<std::int64_t>();

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace listing_service
