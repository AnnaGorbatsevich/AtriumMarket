#include "add_product_proxy.hpp"

#include <chrono>

#include <fmt/format.h>

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

constexpr std::string_view kRequiredFields[] = {"name", "variants"};

void ValidateAddProductPayload(const userver::formats::json::Value& payload) {
    for (const auto field : kRequiredFields) {
        const auto& value = payload[std::string{field}];
        if (value.IsMissing() || value.IsNull()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{fmt::format("Missing required field: {}", field)}
            );
        }
    }
    if (!payload["variants"].IsArray() || payload["variants"].GetSize() == 0) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"variants must be a non-empty array"}
        );
    }
}

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"POST, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Authorization, Content-Type"});
}

}  // namespace

AddProductProxyHandler::AddProductProxyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      http_requests_(context) {}

std::string AddProductProxyHandler::HandleRequestThrow(
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
    ValidateAddProductPayload(payload);

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
            userver::server::handlers::ExternalBody{"Only sellers can add products"}
        );
    }

    userver::formats::json::ValueBuilder forwarded_payload{payload};
    forwarded_payload["sellerId"] = identity["id"].As<std::int64_t>();

    try {
        auto upstream_response = http_requests_.Post(ListingServiceUrl(), "/add_product", 
            {{"Content-Type", "application/json"}}, 2000,
            userver::formats::json::ToString(forwarded_payload.ExtractValue()));

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
