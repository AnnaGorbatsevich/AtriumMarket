#include "variant.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/storages/postgres/component.hpp>

namespace listing_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

}  // namespace

VariantHandler::VariantHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      db_dao_(context) {}

std::string VariantHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto& http_response = request.GetHttpResponse();
    SetCorsHeaders(http_response);

    if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
        return {};
    }

    if (!request.HasArg("variantId")) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Missing required query parameter: variantId"}
        );
    }

    std::int64_t variant_id = 0;
    try {
        variant_id = std::stoll(request.GetArg("variantId"));
    } catch (const std::exception&) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"variantId must be an integer"}
        );
    }

    auto result = db_dao_.GetVariant(variant_id);
    if (result.IsEmpty()) {
        throw userver::server::handlers::CustomHandlerException(
            userver::server::handlers::HandlerErrorCode::kResourceNotFound,
            userver::server::handlers::ExternalBody{"Variant not found"}
        );
    }

    const auto row = result[0];
    userver::formats::json::ValueBuilder response_body;
    response_body["id"] = row["id"].As<std::int64_t>();
    response_body["sellerId"] = row["seller_id"].As<std::int64_t>();
    response_body["price"] = row["price"].As<int>();
    response_body["quantity"] = row["quantity"].As<int>();

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace listing_service
