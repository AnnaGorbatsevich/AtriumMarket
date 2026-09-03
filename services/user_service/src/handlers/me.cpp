#include "me.hpp"
#include "security.hpp"

#include <string_view>

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/storages/postgres/component.hpp>

namespace user_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Authorization, Content-Type"});
}

constexpr std::string_view kBearerPrefix = "Bearer ";

constexpr std::string_view kSelectUserByEmailQuery = R"~(
SELECT full_name, role::text FROM users WHERE email = $1
)~";

std::string ExtractBearerToken(const userver::server::http::HttpRequest& request) {
    const auto& auth_header = request.GetHeader("Authorization");
    if (!auth_header.starts_with(kBearerPrefix)) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Missing bearer token"}
        );
    }
    return auth_header.substr(kBearerPrefix.size());
}

}  // namespace

MeHandler::MeHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

std::string MeHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto& http_response = request.GetHttpResponse();
    SetCorsHeaders(http_response);

    if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
        return {};
    }

    const auto token = ExtractBearerToken(request);

    std::string email;
    try {
        email = decode_jwt_token(token);
    } catch (const std::exception&) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Invalid or expired token"}
        );
    }

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectUserByEmailQuery}},
        email
    );

    if (result.IsEmpty()) {
        throw userver::server::handlers::Unauthorized(userver::server::handlers::ExternalBody{"User not found"});
    }

    userver::formats::json::ValueBuilder response_body;
    response_body["email"] = email;
    response_body["fullName"] = result[0]["full_name"].As<std::string>();
    response_body["role"] = result[0]["role"].As<std::string>();

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace user_service
