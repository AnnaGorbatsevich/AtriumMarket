#include "checkout.hpp"

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

}  // namespace

CheckoutHandler::CheckoutHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      db_dao_(context) {}

std::string CheckoutHandler::HandleRequestThrow(
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

    const auto& buyer_id = payload["buyerId"];
    if (buyer_id.IsMissing() || buyer_id.IsNull()) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Missing required field: buyerId"}
        );
    }

    const auto ordered = db_dao_.Checkout(buyer_id.As<std::int64_t>());
    if (ordered == 0) {
        throw userver::server::handlers::ClientError(userver::server::handlers::ExternalBody{"Cart is empty"});
    }

    userver::formats::json::ValueBuilder response_body;
    response_body["status"] = "ok";
    response_body["orders"] = static_cast<std::int64_t>(ordered);

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace order_service
