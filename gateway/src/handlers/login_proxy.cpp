#include "login_proxy.hpp"

#include <atomic>

#include <fmt/format.h>

#include <userver/formats/json.hpp>
#include <userver/http/content_type.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>

namespace gateway {

namespace {

constexpr std::string_view kRequiredFields[] = {"email", "password"};

void ValidateLoginPayload(const userver::formats::json::Value& payload) {
    for (const auto field : kRequiredFields) {
        const auto value = payload[std::string{field}].As<std::string>({});
        if (value.empty()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{fmt::format("Missing required field: {}", field)}
            );
        }
    }
}

userver::formats::json::Value MockAuthServiceLogin(const userver::formats::json::Value& payload) {
    static std::atomic<std::uint64_t> next_id{1};

    const auto password = payload["password"].As<std::string>();
    const auto email = payload["email"].As<std::string>();

    userver::formats::json::ValueBuilder result;
    result["status"] = "ok";
    result["email"] = email;
    return result.ExtractValue();
}


void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, POST, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

}  // namespace

std::string LoginProxyHandler::HandleRequestThrow(
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

    ValidateLoginPayload(payload);

    const auto result = MockAuthServiceLogin(payload);

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(result);
}

}  // namespace gateway
