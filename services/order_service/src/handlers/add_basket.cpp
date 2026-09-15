#include "add_basket.hpp"

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
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"POST, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

constexpr std::string_view kRequiredFields[] = {"sellerId", "buyerId", "variadId", "quantity", "price"};
constexpr std::string_view kAllowedStatuses[] = {"cart", "ordered", "accepted", "shipped", "ready_for_pickup", "received", "cancelled"};

void ValidatePayload(const userver::formats::json::Value& payload) {
    for (const auto field : kRequiredFields) {
        const auto& value = payload[std::string{field}];
        if (value.IsMissing() || value.IsNull()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{fmt::format("Missing required field: {}", field)}
            );
        }
    }

    const auto status = payload["status"].As<std::optional<std::string>>();
    if (status.has_value()) {
        const auto* allowed_end = kAllowedStatuses + std::size(kAllowedStatuses);
        if (std::find(kAllowedStatuses, allowed_end, *status) == allowed_end) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{"invalid status"}
            );
        }
    }
}

}  // namespace

AddBasketHandler::AddBasketHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      db_dao_(context) {}

std::string AddBasketHandler::HandleRequestThrow(
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

    db_dao_.InsertOrder(payload);

    userver::formats::json::ValueBuilder response_body;
    response_body["status"] = "ok";

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace order_service
