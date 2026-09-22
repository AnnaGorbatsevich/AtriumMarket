#include "update_order_status.hpp"
#include "order_status.hpp"

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

constexpr std::string_view kRequiredFields[] = {"orderId", "actorId", "actorRole", "status"};

void ValidatePayload(const userver::formats::json::Value& payload) {
    for (const auto field : kRequiredFields) {
        const auto& value = payload[std::string{field}];
        if (value.IsMissing() || value.IsNull()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{fmt::format("Missing required field: {}", field)}
            );
        }
    }

    const auto role = payload["actorRole"].As<std::string>({});
    if (role != "seller" && role != "buyer") {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"actorRole must be seller or buyer"}
        );
    }
    if (!IsOrderStatus(payload["status"].As<std::string>({}))) {
        throw userver::server::handlers::ClientError(userver::server::handlers::ExternalBody{"Invalid status"});
    }
}

}  // namespace

UpdateOrderStatusHandler::UpdateOrderStatusHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      db_dao_(context),
      event_publisher_(context) {}

std::string UpdateOrderStatusHandler::HandleRequestThrow(
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

    const auto change = db_dao_.ChangeOrderStatus(
        payload["orderId"].As<std::int64_t>(),
        payload["actorId"].As<std::int64_t>(),
        payload["actorRole"].As<std::string>(),
        payload["status"].As<std::string>()
    );

    switch (change.result) {
        case OrderDAO::StatusChange::kNotFound:
            throw userver::server::handlers::CustomHandlerException(
                userver::server::handlers::HandlerErrorCode::kResourceNotFound,
                userver::server::handlers::ExternalBody{"Order not found"}
            );
        case OrderDAO::StatusChange::kTransitionNotAllowed:
            throw userver::server::handlers::ConflictError(
                userver::server::handlers::ExternalBody{"This status change is not allowed"}
            );
        case OrderDAO::StatusChange::kChanged:
            event_publisher_.Publish(*change.event);
            break;
    }

    userver::formats::json::ValueBuilder response_body;
    response_body["status"] = "ok";

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace order_service
