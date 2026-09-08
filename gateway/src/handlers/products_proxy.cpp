#include "products_proxy.hpp"

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

ProductsProxyHandler::ProductsProxyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      http_requests_(context) {}

std::string ProductsProxyHandler::HandleRequestThrow(
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

    if (identity["role"].As<std::string>({}) != "seller") {
        throw userver::server::handlers::CustomHandlerException(
            userver::server::handlers::HandlerErrorCode::kForbidden,
            userver::server::handlers::ExternalBody{"Only sellers can view their products"}
        );
    }

    const auto seller_id = identity["id"].As<std::int64_t>();

    try {
        auto upstream_response =
            http_requests_.Get(ListingServiceUrl(), "/products?sellerId=" + std::to_string(seller_id), {}, 2000);

        http_response.SetStatus(static_cast<userver::server::http::HttpStatus>(upstream_response->status_code()));
        http_response.SetContentType(userver::http::content_type::kApplicationJson);
        return upstream_response->body();
    } catch (const userver::clients::http::BaseException&) {
        throw userver::server::handlers::CustomHandlerException(
            userver::server::handlers::HandlerErrorCode::kBadGateway,
            userver::server::handlers::ExternalBody{"listing-service is unavailable"}
        );
    }
}

}  // namespace gateway
