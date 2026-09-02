#include "register.hpp"
#include "security.hpp"

#include <optional>

#include <fmt/format.h>

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/exceptions.hpp>

namespace user_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, POST, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

constexpr std::string_view kRequiredFields[] = {"role", "fullName", "email", "phone", "password"};

void ValidateRegistrationPayload(const userver::formats::json::Value& payload) {
    for (const auto field : kRequiredFields) {
        if (payload[std::string{field}].As<std::string>({}).empty()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{fmt::format("Missing required field: {}", field)}
            );
        }
    }
}

std::optional<std::string> OptionalField(const userver::formats::json::Value& payload, std::string_view field) {
    auto value = payload[std::string{field}].As<std::string>({});
    if (value.empty()) return std::nullopt;
    return value;
}

constexpr std::string_view kInsertUserQuery = R"~(
INSERT INTO users (role, full_name, company_name, email, phone, tax_id, address, description, password_hash)
VALUES ($1::user_role, $2, $3, $4, $5, $6, $7, $8, $9)
)~";

}  // namespace

RegisterHandler::RegisterHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

std::string RegisterHandler::HandleRequestThrow(
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

    const auto email = payload["email"].As<std::string>();
    const auto password_hash = get_password_hash(payload["password"].As<std::string>());

    try {
        pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::Query{std::string{kInsertUserQuery}},
            payload["role"].As<std::string>(),
            payload["fullName"].As<std::string>(),
            OptionalField(payload, "companyName"),
            email,
            payload["phone"].As<std::string>(),
            OptionalField(payload, "taxId"),
            OptionalField(payload, "address"),
            OptionalField(payload, "description"),
            password_hash
        );
    } catch (const userver::storages::postgres::UniqueViolation&) {
        throw userver::server::handlers::ClientError(userver::server::handlers::ExternalBody{"Email already registered"}
        );
    }

    const auto jwt_token = create_jwt_token(email);

    userver::formats::json::ValueBuilder response_body;
    response_body["status"] = "ok";
    response_body["token"] = jwt_token;

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace user_service
