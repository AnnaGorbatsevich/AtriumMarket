#include "register_proxy.hpp"

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

constexpr std::string_view kRequiredFields[] = {"role", "fullName", "email", "phone", "password"};

void ValidateRegistrationPayload(const userver::formats::json::Value& payload) {
    for (const auto field : kRequiredFields) {
        const auto value = payload[std::string{field}].As<std::string>({});
        if (value.empty()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{fmt::format("Missing required field: {}", field)}
            );
        }
    }
}

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, POST, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

}  // namespace

RegisterProxyHandler::RegisterProxyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      http_client_(context.FindComponent<userver::components::HttpClient>().GetHttpClient()) {}

std::string RegisterProxyHandler::HandleRequestThrow(
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
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Invalid JSON body"}
        );
    }

    ValidateRegistrationPayload(payload);

    try {
        auto upstream_response = http_client_.CreateRequest()
                                      .post(UserServiceUrl() + "/register", request.RequestBody())
                                      .headers({{"Content-Type", "application/json"}})
                                      .timeout(std::chrono::milliseconds(2000))
                                      .perform();

        http_response.SetStatus(static_cast<userver::server::http::HttpStatus>(upstream_response->status_code()));
        http_response.SetContentType(userver::http::content_type::kApplicationJson);
        return upstream_response->body();
    } catch (const userver::clients::http::BaseException&) {
        throw userver::server::handlers::CustomHandlerException(
            userver::server::handlers::HandlerErrorCode::kBadGateway,
            userver::server::handlers::ExternalBody{"user-service is unavailable"}
        );
    }
}

}  // namespace gateway
