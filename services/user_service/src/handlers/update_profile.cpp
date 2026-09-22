#include "update_profile.hpp"
#include "profile.hpp"

#include <optional>

#include <fmt/format.h>

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/optional.hpp>

namespace user_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"POST, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Authorization, Content-Type"});
}

constexpr std::string_view kRequiredFields[] = {"fullName", "phone"};

void ValidatePayload(const userver::formats::json::Value& payload) {
    for (const auto field : kRequiredFields) {
        if (payload[std::string{field}].As<std::string>({}).empty()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{fmt::format("Missing required field: {}", field)}
            );
        }
    }
}

// An optional field that is absent from the payload keeps its stored value;
// one sent as an empty string is cleared.
std::optional<std::string> MergedField(
    const userver::formats::json::Value& payload,
    const userver::storages::postgres::Row& current,
    const std::string& json_key,
    const std::string& column
) {
    const auto& value = payload[json_key];
    if (value.IsMissing()) {
        return current[column].As<std::optional<std::string>>();
    }
    auto text = value.As<std::string>({});
    if (text.empty()) return std::nullopt;
    return text;
}

}  // namespace

UpdateProfileHandler::UpdateProfileHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      db_dao_(context) {}

std::string UpdateProfileHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto& http_response = request.GetHttpResponse();
    SetCorsHeaders(http_response);

    if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
        return {};
    }

    const auto email = AuthenticatedEmail(request);

    userver::formats::json::Value payload;
    try {
        payload = userver::formats::json::FromString(request.RequestBody());
    } catch (const userver::formats::json::Exception&) {
        throw userver::server::handlers::ClientError(userver::server::handlers::ExternalBody{"Invalid JSON body"});
    }
    ValidatePayload(payload);

    auto current = db_dao_.GetMe(email);
    if (current.IsEmpty()) {
        throw userver::server::handlers::Unauthorized(userver::server::handlers::ExternalBody{"User not found"});
    }
    const auto current_row = current[0];

    db_dao_.UpdateProfile(
        email,
        payload["fullName"].As<std::string>(),
        payload["phone"].As<std::string>(),
        MergedField(payload, current_row, "companyName", "company_name"),
        MergedField(payload, current_row, "taxId", "tax_id"),
        MergedField(payload, current_row, "address", "address"),
        MergedField(payload, current_row, "description", "description")
    );

    auto updated = db_dao_.GetMe(email);
    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(ProfileToJson(email, updated[0]));
}

}  // namespace user_service
