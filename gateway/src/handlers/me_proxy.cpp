#include "me_proxy.hpp"

#include <chrono>

#include <userver/clients/http/component.hpp>
#include <userver/clients/http/error.hpp>
#include <userver/components/component_context.hpp>
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

MeProxyHandler::MeProxyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      http_client_(context.FindComponent<userver::components::HttpClient>().GetHttpClient()) {}

std::string MeProxyHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto& http_response = request.GetHttpResponse();
    SetCorsHeaders(http_response);

    if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
        return {};
    }

    try {
        auto upstream_response = http_client_.CreateRequest()
                                      .get(UserServiceUrl() + "/me")
                                      .headers({{"Authorization", request.GetHeader("Authorization")}})
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
